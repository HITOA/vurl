#pragma once

#include <vurl/pass.hpp>
#include <vurl/render_graph_def.hpp>
#include <vurl/resource.hpp>
#include <vurl/texture.hpp>
#include <vurl/buffer.hpp>
#include <vurl/graphics_pipeline.hpp>
#include <vector>
#include <string>
#include <functional>

namespace Vurl {
    class RenderGraph;
    
    class GraphicsPass : public Pass, public HashedObject {
    public:
        GraphicsPass() = delete;
        GraphicsPass(const std::string& name, std::shared_ptr<GraphicsPipeline> pipeline, RenderGraph* graph);
        ~GraphicsPass() = default;

        inline PassType GetPassType() const override { return PassType::Graphics; }

        void AddColorAttachment(std::shared_ptr<Resource<Texture>> texture);
        void AddInputAttachment(std::shared_ptr<Resource<Texture>> texture);
        void SetClearAttachment(std::shared_ptr<Resource<Texture>> texture);
        
        void AddBufferInput(std::shared_ptr<Resource<Buffer>> buffer);

        inline uint32_t GetColorAttachmentCount() const { return colorAttachments.size(); }
        inline uint32_t GetInputAttachmentCount() const { return inputAttachments.size(); }
        inline TextureHandle GetColorAttachment(uint32_t idx) const { return colorAttachments[idx]; }
        inline TextureHandle GetInputAttachment(uint32_t idx) const { return inputAttachments[idx]; }
        inline TextureHandle GetClearAttachment() const { return clearAttachment; }

        inline std::shared_ptr<GraphicsPipeline> GetGraphicsPipeline() const { return graphicsPipeline; }

        inline void SetRenderingCallback(std::function<void(VkCommandBuffer, uint32_t)> callback) { renderingCallback = callback; }
        inline std::function<void(VkCommandBuffer, uint32_t)> GetRenderingCallback() const { return renderingCallback; }
        
        inline uint32_t GetHash() const {
            Hasher hasher{};
            hasher.U32(graphicsPipeline->GetHash());
            for (uint32_t i = 0; i < colorAttachments.size(); ++i)
                hasher.U32(colorAttachments[i]);
            for (uint32_t i = 0; i < inputAttachments.size(); ++i)
                hasher.U32(inputAttachments[i]);
            hasher.U32(clearAttachment);
            return hasher.Get();
        };

    private:
        std::string name;
        std::shared_ptr<GraphicsPipeline> graphicsPipeline = nullptr;
        RenderGraph* graph;

        std::vector<TextureHandle> colorAttachments{};
        std::vector<TextureHandle> inputAttachments{};
        TextureHandle clearAttachment = VURL_NULL_HANDLE;

        std::vector<BufferHandle> inputBuffers{};
        
        std::function<void(VkCommandBuffer, uint32_t)> renderingCallback{};
    };
}