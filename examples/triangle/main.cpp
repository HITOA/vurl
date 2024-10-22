#include <memory>
#include <iostream>
#include <vector>
#include <fstream>

#include <example_base.hpp>

struct Vertex {
    float position[2];
    float color[3];
};

class TriangleExample : public ExampleBase {
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
            { 0, Vurl::VertexInputAttributeFormat::Vector2 }, //Position
            { 1, Vurl::VertexInputAttributeFormat::Vector3 }  //Color
        };

        std::shared_ptr<Vurl::GraphicsPipeline> graphicsPipeline = std::make_shared<Vurl::GraphicsPipeline>(context->GetDevice());
        graphicsPipeline->SetVertexShader(vertShader);
        graphicsPipeline->SetFragmentShader(fragShader);
        graphicsPipeline->AddVertexInput(inputDescription);
        graphicsPipeline->CreatePipelineLayout();

        graph = std::make_shared<Vurl::RenderGraph>(context);
        graph->CreatePipelineCache();
        graph->CreateTransientCommandPool();
        graph->SetSurface(surface);

        std::shared_ptr<Vurl::Resource<Vurl::Buffer>> vertexBuffer = graph->CreateBuffer<Vurl::Resource<Vurl::Buffer>>("Triangle Vertex Buffer", false);
        std::shared_ptr<Vurl::Buffer> vertexBufferSlice = std::make_shared<Vurl::Buffer>();
        vertexBuffer->SetSliceCount(1);
        vertexBuffer->SetResourceSlice(vertexBufferSlice, 0);

        Vertex vertices[3] = { 
            { { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
        };

        vertexBufferSlice->size = sizeof(vertices);
        vertexBufferSlice->usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        
        graph->CommitBuffer(vertexBuffer, (const uint8_t*)vertices, sizeof(vertices));

        std::shared_ptr<Vurl::GraphicsPass> pass = graph->CreateGraphicsPass("Triangle Pass", graphicsPipeline);
        pass->AddColorAttachment(surface->GetBackBuffer());
        pass->SetClearAttachment(surface->GetBackBuffer());
        pass->AddBufferInput(vertexBuffer);
        pass->SetRenderingCallback([](VkCommandBuffer commandBuffer){
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        });

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

private:
    std::shared_ptr<Vurl::RenderingContext> context = nullptr;
    std::shared_ptr<Vurl::Surface> surface = nullptr;
    std::shared_ptr<Vurl::RenderGraph> graph = nullptr;
};

int main() {
    TriangleExample example{};
    example.Run();
    return 0;
}