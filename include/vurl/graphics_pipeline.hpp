#pragma once

#include <volk.h>
#include <vurl/shader.hpp>
#include <vurl/hash.hpp>
#include <memory>
#include <vector>

namespace Vurl {
    class GraphicsPipeline : public HashedObject {
    public:
        GraphicsPipeline() = delete;
        GraphicsPipeline(VkDevice device) : vkDevice{ device } {}
        ~GraphicsPipeline() = default;

        bool CreatePipelineLayout();
        void DestroyPipelineLayout();
        inline VkPipelineLayout GetPipelineLayout() const { return vkPipelineLayout; }

        inline void SetVertexShader(std::shared_ptr<Shader> shader) { vertexShader = shader; }
        inline std::shared_ptr<Shader> GetVertexShader() const { return vertexShader; }

        inline void SetFragmentShader(std::shared_ptr<Shader> shader) { fragmentShader = shader; }
        inline std::shared_ptr<Shader> GetFragmentShader() const { return fragmentShader; }

        inline void SetTessellationControlShader(std::shared_ptr<Shader> shader) { tessellationControlShader = shader; }
        inline std::shared_ptr<Shader> GetTessellationControlShader() const { return tessellationControlShader; }
        
        inline void SetTessellationEvaluationShader(std::shared_ptr<Shader> shader) { tessellationEvaluationShader = shader; }
        inline std::shared_ptr<Shader> GetTessellationEvaluationShader() const { return tessellationEvaluationShader; }
        
        inline void SetGeometryShader(std::shared_ptr<Shader> shader) { geometryShader = shader; }
        inline std::shared_ptr<Shader> GetGeometryShader() const { return geometryShader; }

        inline uint32_t GetDynamicStatesCount() const { return (uint32_t)dynamicStates.size(); }
        inline const VkDynamicState* GetDynamicStates() const { return dynamicStates.data(); }

        inline void SetPipelinePrimitiveTopology(VkPrimitiveTopology topology) { vkPrimitiveTopology = topology; }
        inline VkPrimitiveTopology GetPipelinePrimitiveTopology() const { return vkPrimitiveTopology; }

        inline void SetPipelineCullMode(VkCullModeFlagBits cullMode) { vkCullMode = cullMode; }
        inline VkCullModeFlagBits GetPipelineCullMode() const { return vkCullMode; }

        inline uint32_t GetHash() const {
            Hasher hasher{};
            hasher.Ptr(vertexShader.get());
            hasher.Ptr(fragmentShader.get());
            hasher.Ptr(tessellationControlShader.get());
            hasher.Ptr(tessellationEvaluationShader.get());
            hasher.Ptr(geometryShader.get());
            hasher.Ptr(vkPipelineLayout);
            for (uint32_t i = 0; i < dynamicStates.size(); ++i)
                hasher.U32((uint32_t)dynamicStates[i]);
            hasher.U32(vkPrimitiveTopology);
            hasher.U32(vkCullMode);
            return hasher.Get();
        };

    private:
        VkDevice vkDevice = VK_NULL_HANDLE;

        std::shared_ptr<Shader> vertexShader = nullptr;
        std::shared_ptr<Shader> fragmentShader = nullptr;
        std::shared_ptr<Shader> tessellationControlShader = nullptr;
        std::shared_ptr<Shader> tessellationEvaluationShader = nullptr;
        std::shared_ptr<Shader> geometryShader = nullptr;

        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

        std::vector<VkDynamicState> dynamicStates{};
        VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
    };
}