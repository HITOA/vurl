#include <memory>
#include <iostream>
#include <vector>
#include <functional>

#include <example_base.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct TransformationData {
    glm::mat4 mvp;
};

struct Vertex {
    float position[3];
    float normal[3];
};

class ForwardSuzanneExample : public ExampleBase {
public:
    virtual void Initialize() override {
        context = std::make_shared<Vurl::RenderingContext>();
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        context->CreateInstance(nullptr, glfwExtensions, glfwExtensionCount, true);

        surface = std::make_shared<Vurl::Surface>(CreateWSI());
        surface->CreateSurface(context->GetInstance());

        context->CreateDevice(surface->GetSurfaceKHR());

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        surface->CreateSwapchain(context->GetPhysicalDevice(), context->GetDevice(), (uint32_t)width, (uint32_t)height);

        std::shared_ptr<Vurl::Shader> vertShader = CreateShaderFromFile("vert.spv", context->GetDevice());
        std::shared_ptr<Vurl::Shader> fragShader = CreateShaderFromFile("frag.spv", context->GetDevice());

        Vurl::VertexInputDescription inputDescription{
            { 0, Vurl::VertexInputAttributeFormat::Vector3 }, //Position
            { 1, Vurl::VertexInputAttributeFormat::Vector3 }  //Normal
        };

        graphicsPipeline = std::make_shared<Vurl::GraphicsPipeline>(context->GetDevice());
        graphicsPipeline->SetVertexShader(vertShader);
        graphicsPipeline->SetFragmentShader(fragShader);
        graphicsPipeline->AddVertexInput(inputDescription);
        graphicsPipeline->AddPushConstantRange<TransformationData>(VK_SHADER_STAGE_VERTEX_BIT);
        graphicsPipeline->CreatePipelineLayout();

        graph = std::make_shared<Vurl::RenderGraph>(context);
        graph->CreatePipelineCache();
        graph->CreateTransientCommandPool();
        graph->SetSurface(surface);

        vertexBuffer = LoadMesh("suzanne.obj", graph);

        std::shared_ptr<Vurl::GraphicsPass> pass = graph->CreateGraphicsPass("Triangle Pass", graphicsPipeline);
        pass->AddColorAttachment(surface->GetBackBuffer());
        pass->SetClearAttachment(surface->GetBackBuffer());
        pass->AddBufferInput(vertexBuffer);
        pass->SetRenderingCallback(std::bind(&ForwardSuzanneExample::Draw, this, std::placeholders::_1, std::placeholders::_2));
        
        graph->Build();
    }

    virtual void Update() override {
        graph->Execute();
    }

    virtual void Destroy() override {
        graph->Destroy();
        graph->DestroyTransientCommandPool();
        graph->DestroyPipelineCache();
        surface->DestroySwapchain();
        surface->DestroySurface();
        context->DestroyDevice();
        context->DestroyInstance();
    }

    std::shared_ptr<Vurl::Resource<Vurl::Buffer>> LoadMesh(const std::string& filename, std::shared_ptr<Vurl::RenderGraph> graph) {
        tinyobj::ObjReaderConfig readerConfig{};
        readerConfig.mtl_search_path = "./";

        tinyobj::ObjReader reader{};
        if (!reader.ParseFromFile(filename, readerConfig)) {
            return nullptr;
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        
        std::vector<Vertex> vertices{};

        for (size_t s = 0; s < shapes.size(); ++s) {
            size_t indexOffset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
                size_t fv = 3;
                for (size_t v = 0; v < fv; ++v) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[indexOffset + v];
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

                    Vertex vertex{};
                    vertex.position[0] = vx;
                    vertex.position[1] = vy;
                    vertex.position[2] = vz;
                    vertex.normal[0] = nx;
                    vertex.normal[1] = ny;
                    vertex.normal[2] = nz;
                    vertices.push_back(vertex);
                }
                indexOffset += fv;
            }
        }

        std::shared_ptr<Vurl::Resource<Vurl::Buffer>> buffer = graph->CreateBuffer<Vurl::Resource<Vurl::Buffer>>("Vertex Buffer", false);
        std::shared_ptr<Vurl::Buffer> slice = std::make_shared<Vurl::Buffer>();
        buffer->SetSliceCount(1);
        buffer->SetResourceSlice(slice, 0);

        slice->size = sizeof(Vertex) * vertices.size();
        slice->usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        
        graph->CommitBuffer(buffer, (const uint8_t*)vertices.data(), sizeof(Vertex) * vertices.size());

        return buffer;
    }

    void Draw(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
        TransformationData transformationData{};
        
        glm::mat4 view = glm::translate(glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, -5.0f ));
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)surface->GetWidth() / (float)surface->GetHeight(), 0.1f, 1000.0f);
        glm::mat4 model = glm::rotate(glm::mat4( 1.0f ), glm::radians((float)frameIndex * 0.5f), glm::vec3( 0.0f, 1.0f, 0.0f ));

        transformationData.mvp = projection * view * model;

        vkCmdPushConstants(commandBuffer, graphicsPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(TransformationData), &transformationData);

        VkBuffer vertexBuffers[] = { vertexBuffer->GetResourceSlice(frameIndex)->vkBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(commandBuffer, vertexBuffer->GetResourceSlice(frameIndex)->size / sizeof(Vertex), 1, 0, 0);
    }

private:
    std::shared_ptr<Vurl::RenderingContext> context = nullptr;
    std::shared_ptr<Vurl::Surface> surface = nullptr;
    std::shared_ptr<Vurl::RenderGraph> graph = nullptr;
    std::shared_ptr<Vurl::GraphicsPipeline> graphicsPipeline = nullptr;
    std::shared_ptr<Vurl::Resource<Vurl::Buffer>> vertexBuffer = nullptr;
};

int main() {
    ForwardSuzanneExample example{};
    example.Run();
    return 0;
}