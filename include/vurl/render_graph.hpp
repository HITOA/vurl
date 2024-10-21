#pragma once

#include <volk.h>
#include <vurl/pass.hpp>
#include <vurl/graphics_pass.hpp>
#include <vurl/render_graph_def.hpp>
#include <vurl/graphics_pipeline.hpp>
#include <vurl/resource.hpp>
#include <vurl/texture.hpp>
#include <vurl/rendering_context.hpp>
#include <vurl/surface.hpp>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Vurl {
    class RenderGraph {
    private:
        struct GraphicsPassGroup {
            std::vector<std::shared_ptr<GraphicsPass>> passes{};
            std::unordered_map<TextureHandle, VkAttachmentDescription> attachmentDescriptions{};
            std::vector<VkPipeline> pipelines{};
            std::vector<VkFramebuffer> framebuffers{};
            VkRenderPass vkRenderPass = VK_NULL_HANDLE;
            VkViewport viewport{};
            VkRect2D scissor{};
            uint32_t minSwapchainColorAttachmentSubpassIndex = -1;
        };

    public:
        RenderGraph() = delete;
        RenderGraph(std::shared_ptr<RenderingContext> context);
        ~RenderGraph();
        
        void SetSurface(std::shared_ptr<Surface> surface);

        TextureHandle GetTextureHandle(std::shared_ptr<Resource<Texture>> texture);
        void AddExternalTexture(std::shared_ptr<Resource<Texture>> texture);

        template<typename T>
        std::shared_ptr<T> CreateTexture(const std::string& name, bool transient = true) {
            std::shared_ptr<T> texture = std::make_shared<T>(name);
            texture->SetTransient(transient);
            texture->SetExternal(false);

            textures.push_back(texture);

            return texture;
        }
        
        std::shared_ptr<GraphicsPass> CreateGraphicsPass(const std::string& name, std::shared_ptr<GraphicsPipeline> pipeline);

        void Build();
        void Destroy();
        void Execute();

        void CreatePipelineCache();
        void DestroyPipelineCache();
    private:
        bool BuildDirectedPassesGraph();
        bool BuildGraphicsPassGroups();
        bool BuildGraphicsPassGroup(uint32_t firstPass);
        bool BuildGraphicsPassGroupRenderPass(GraphicsPassGroup* group);
        bool BuildGraphicsPassGroupFramebuffers(GraphicsPassGroup* group);
        bool BuildGraphicsPassGroupGraphicsPipelines(GraphicsPassGroup* group);
        bool BuildCommandBuffers();
        bool BuildSynchronizationObjects();

        void DestroyGraphicsPassGroups();
        void DestroyCommandBuffers();
        void DestroySynchronizationObjects();

        bool ExecuteGraphicsPassGroup(GraphicsPassGroup* group, VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex);

    private:
        bool complete = false;

        std::shared_ptr<RenderingContext> context = nullptr;
        uint64_t frameIndex = 0;

        std::shared_ptr<Surface> surface = nullptr;
        
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer primaryCommandBuffers[VURL_MAX_FRAMES_IN_FLIGHT];
        VkSemaphore availableSwapchainImageSemaphores[VURL_MAX_FRAMES_IN_FLIGHT];
        VkSemaphore renderFinishedSemaphores[VURL_MAX_FRAMES_IN_FLIGHT];
        VkFence inFlightFences[VURL_MAX_FRAMES_IN_FLIGHT];

        std::vector<std::shared_ptr<Resource<Texture>>> textures{};
        TextureHandle backBufferTexture = VURL_NULL_HANDLE;

        std::vector<std::shared_ptr<Pass>> passes{};
        std::vector<std::vector<uint32_t>> culledDirectedPassesGraph{};
        std::vector<uint32_t> beginPasses{};

        std::vector<GraphicsPassGroup> graphicsPassGroups{};
    };
}