#include "scene.hpp"

#include <fstream>
#include <vector>
#include <functional>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>


std::shared_ptr<Vurl::Shader> CreateShaderFromFile(const std::string& path, VkDevice device) {
    std::vector<char> buffer{};
    std::ifstream file{ path, std::ios::ate | std::ios::binary };
    if (!file.is_open())
        return nullptr;

    size_t size = (size_t)file.tellg();
    file.seekg(0);
    buffer.resize(size);
    file.read(buffer.data(), size);
    file.close();

    std::shared_ptr<Vurl::Shader> shader = std::make_shared<Vurl::Shader>(device);
    shader->CreateShaderModule((const uint32_t*)buffer.data(), buffer.size());

    return shader;
}

void Scene::Initialize() {
    graph = std::make_shared<Vurl::RenderGraph>(context);
    graph->CreatePipelineCache();
    graph->CreateTransientCommandPool();
    graph->SetSurface(surface);

    //Mehs input description
    Vurl::VertexInputDescription meshInputDescription{
        { 0, Vurl::VertexInputAttributeFormat::Vector3 }, //Position
        { 1, Vurl::VertexInputAttributeFormat::Vector3 }  //Normal
    };

    //Build all graphics pipelines
    gPassPipeline = std::make_shared<Vurl::GraphicsPipeline>(context->GetDevice());
    std::shared_ptr<Vurl::Shader> gPassVertexShader = CreateShaderFromFile("shaders/gpass_vert.spv", context->GetDevice());
    std::shared_ptr<Vurl::Shader> gPassFragmentShader = CreateShaderFromFile("shaders/gpass_frag.spv", context->GetDevice());
    gPassPipeline->SetVertexShader(gPassVertexShader);
    gPassPipeline->SetFragmentShader(gPassFragmentShader);
    gPassPipeline->SetPipelineCullMode(VK_CULL_MODE_NONE);
    gPassPipeline->AddVertexInput(meshInputDescription);
    gPassPipeline->AddPushConstantRange<MeshPushConstant>(VK_SHADER_STAGE_VERTEX_BIT);
    gPassPipeline->CreatePipelineLayout();

    lightingPassPipeline = std::make_shared<Vurl::GraphicsPipeline>(context->GetDevice());
    std::shared_ptr<Vurl::Shader> lightingPassVertexShader = CreateShaderFromFile("shaders/lighting_vert.spv", context->GetDevice());
    std::shared_ptr<Vurl::Shader> lightingPassFragmentShader = CreateShaderFromFile("shaders/lighting_frag.spv", context->GetDevice());
    lightingPassPipeline->SetVertexShader(lightingPassVertexShader);
    lightingPassPipeline->SetFragmentShader(lightingPassFragmentShader);
    lightingPassPipeline->SetPipelineCullMode(VK_CULL_MODE_NONE);
    lightingPassPipeline->CreatePipelineLayout();

    //Create resources
    std::shared_ptr<Vurl::Resource<Vurl::Texture>> depthStencilTarget = graph->CreateTexture<Vurl::Resource<Vurl::Texture>>("Depth Stencil Target", false);
    std::shared_ptr<Vurl::Texture> depthStencilTargetSlice = std::make_shared<Vurl::Texture>();
    depthStencilTarget->SetSliceCount(1);
    depthStencilTarget->SetResourceSlice(depthStencilTargetSlice, 0);
    depthStencilTargetSlice->vkFormat = VK_FORMAT_D32_SFLOAT;
    depthStencilTargetSlice->sizeClass = Vurl::TextureSizeClass::SwapchainRelative;
    depthStencilTargetSlice->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthStencilTargetSlice->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    graph->CommitTexture(depthStencilTarget);

    std::shared_ptr<Vurl::Resource<Vurl::Texture>> colorTarget = graph->CreateTexture<Vurl::Resource<Vurl::Texture>>("Depth Stencil Target", false);
    std::shared_ptr<Vurl::Texture> colorTargetSlice = std::make_shared<Vurl::Texture>();
    colorTarget->SetSliceCount(1);
    colorTarget->SetResourceSlice(colorTargetSlice, 0);
    colorTargetSlice->vkFormat = VK_FORMAT_B8G8R8A8_SRGB;
    colorTargetSlice->sizeClass = Vurl::TextureSizeClass::SwapchainRelative;
    colorTargetSlice->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    colorTargetSlice->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    graph->CommitTexture(colorTarget);

    std::shared_ptr<Vurl::Resource<Vurl::Texture>> normalTarget = graph->CreateTexture<Vurl::Resource<Vurl::Texture>>("Depth Stencil Target", false);
    std::shared_ptr<Vurl::Texture> normalTargetSlice = std::make_shared<Vurl::Texture>();
    normalTarget->SetSliceCount(1);
    normalTarget->SetResourceSlice(normalTargetSlice, 0);
    normalTargetSlice->vkFormat = VK_FORMAT_R16G16_SFLOAT;
    normalTargetSlice->sizeClass = Vurl::TextureSizeClass::SwapchainRelative;
    normalTargetSlice->usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    normalTargetSlice->aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    graph->CommitTexture(normalTarget);

    //Build all graphics passes
    std::shared_ptr<Vurl::GraphicsPass> gPass = graph->CreateGraphicsPass("GPass", gPassPipeline);
    gPass->AddColorAttachment(colorTarget);
    gPass->AddColorAttachment(normalTarget);
    gPass->ClearAttachment(0, VkClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f });
    gPass->SetDepthStencilAttachment(depthStencilTarget);
    gPass->SetRenderingCallback(std::bind(&Scene::GPassRenderingCallback, this, std::placeholders::_1, std::placeholders::_2));

    std::shared_ptr<Vurl::GraphicsPass> lightingPass = graph->CreateGraphicsPass("Lighting Pass", lightingPassPipeline);
    lightingPass->AddColorAttachment(surface->GetBackBuffer());
    lightingPass->ClearAttachment(0, VkClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f });
    lightingPass->AddInputAttachment(colorTarget);
    lightingPass->AddInputAttachment(normalTarget);
    lightingPass->SetRenderingCallback(std::bind(&Scene::LightingPassRenderingCallback, this, std::placeholders::_1, std::placeholders::_2));

    graph->Build();
}

void Scene::Uninitialize() {
    graph->Destroy();
    graph->DestroyTransientCommandPool();
    graph->DestroyPipelineCache();
}

void Scene::Draw() {
    view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(90.0f), (float)surface->GetWidth() / (float)surface->GetHeight(), 0.1f, 2000.0f);
    //graph->Execute();
}

void Scene::AddNode(std::shared_ptr<Node> node) {
    nodes.push_back(node);
    ProcessNode(node);
}

void Scene::ProcessNode(std::shared_ptr<Node> node) {
    if (node->GetMesh()) {
        /*std::shared_ptr<Vurl::GraphicsPass> debugForwardPass = graph->GetPassByName<Vurl::GraphicsPass>("Debug Pass");
        std::shared_ptr<Mesh> mesh = node->GetMesh();

        for (uint32_t i = 0; i < mesh->GetPrimitiveCount(); ++i) {
            std::shared_ptr<Primitive> primitive = mesh->GetPrimitive(i);

            debugForwardPass->AddBufferInput(primitive->GetVertexBuffer());
            debugForwardPass->AddBufferInput(primitive->GetIndexBuffer());

            primitives.push_back(primitive);
        }*/
    }

    for (uint32_t i = 0; i < node->GetChildCount(); ++i) {
        ProcessNode(node->GetChild(i));
    }
}

void Scene::GPassRenderingCallback(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
    MeshPushConstant meshPushConstant{};
        
    meshPushConstant.view = view;
    meshPushConstant.projection = projection;
    meshPushConstant.model = glm::mat4( 1.0f );

    meshPushConstant.mvp = meshPushConstant.projection * meshPushConstant.view * meshPushConstant.model;

    vkCmdPushConstants(commandBuffer, gPassPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstant), &meshPushConstant);

    for (uint32_t i = 0; i < primitives.size(); ++i) {
        std::shared_ptr<Vurl::Buffer> vertexBuffer = primitives[i]->GetVertexBuffer()->GetResourceSlice(frameIndex);
        std::shared_ptr<Vurl::Buffer> indexBuffer = primitives[i]->GetIndexBuffer()->GetResourceSlice(frameIndex);

        VkBuffer vertexBuffers[] = { vertexBuffer->vkBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->vkBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, indexBuffer->size / sizeof(uint32_t), 1, 0, 0, 0);
    }
}

void Scene::LightingPassRenderingCallback(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
}
