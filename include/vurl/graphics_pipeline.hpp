#pragma once

#include <volk.h>
#include <vurl/shader.hpp>
#include <vurl/hash.hpp>
#include <memory>
#include <vector>

namespace Vurl {
    enum class VertexInputAttributeFormat {
        Float,
        Vector2,
        Vector3,
        Vector4
    };

    struct VertexInputAttributeDescription {
        uint32_t location = 0;
        uint32_t offset = 0;
        VertexInputAttributeFormat format = VertexInputAttributeFormat::Float;

        VertexInputAttributeDescription() = delete;
        VertexInputAttributeDescription(uint32_t location, VertexInputAttributeFormat format) : 
                location{ location }, format{ format } {}

        inline uint32_t GetSize() const {
            switch (format) {
                case VertexInputAttributeFormat::Float:   return sizeof(float);
                case VertexInputAttributeFormat::Vector2: return sizeof(float) * 2;
                case VertexInputAttributeFormat::Vector3: return sizeof(float) * 3;
                case VertexInputAttributeFormat::Vector4: return sizeof(float) * 4;
                default: return 0;
            }
        }

        inline VkFormat GetVkFormat() const {
            switch (format) {
                case VertexInputAttributeFormat::Float:   return VK_FORMAT_R32_SFLOAT;
                case VertexInputAttributeFormat::Vector2: return VK_FORMAT_R32G32_SFLOAT;
                case VertexInputAttributeFormat::Vector3: return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexInputAttributeFormat::Vector4: return VK_FORMAT_R32G32B32A32_SFLOAT;
                default: return VK_FORMAT_R32_SFLOAT;
            }
        }
    };

    class VertexInputDescription {
    public:
        VertexInputDescription() = default;
        VertexInputDescription(std::initializer_list<VertexInputAttributeDescription> attributes) : attributes{ attributes } {
            for (VertexInputAttributeDescription& description : this->attributes) {
                description.offset = stride;
                stride += description.GetSize();
            }
        }
        ~VertexInputDescription() = default;

        inline uint32_t GetAttributeCount() const { return attributes.size(); }
        inline const VertexInputAttributeDescription& GetAttribute(uint32_t idx) const { return attributes[idx]; }
        inline void AddAttribute(VertexInputAttributeDescription& description) { 
            VertexInputAttributeDescription& a = attributes.emplace_back(description);
            a.offset = stride;
            stride += a.GetSize();
        }

        inline void SetInputRate(VkVertexInputRate rate) { inputRate = rate; }
        inline VkVertexInputRate GetInputRate() const { return inputRate; }

        inline uint32_t GetStride() const { return stride; }
        
    private:
        std::vector<VertexInputAttributeDescription> attributes{};
        VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        uint32_t stride = 0;
    };

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

        inline uint32_t GetVertexInputCount() const { return vertexInputs.size(); }
        inline const VertexInputDescription& GetVertexInput(uint32_t idx) const { return vertexInputs[idx]; }
        inline void AddVertexInput(const VertexInputDescription& inputDescription) { vertexInputs.push_back(inputDescription); }

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

        std::vector<VertexInputDescription> vertexInputs{};
        std::vector<VkDynamicState> dynamicStates{};
        VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
    };
}