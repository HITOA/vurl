#pragma once

#include <vurl/vulkan_header.hpp>
#include <vector>
#include <unordered_map>


namespace Vurl {
    class DescriptorSetAllocator {
    public:
        struct PoolSizes {
			std::vector<std::pair<VkDescriptorType,float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

    public:
        DescriptorSetAllocator() = delete;
        DescriptorSetAllocator(VkDevice device) : vkDevice{ device } {}
        ~DescriptorSetAllocator() { Destroy(); }

        void Reset();
        void Destroy();
        VkDescriptionSet Allocate(VkDescriptionSetLayout layout);
        void Allocate(VkDescriptionSet* descriptionSets, VkDescriptionSetLayout* layouts, uint32_t count);

    private:
        VkDescriptorPool GetPool();
        VkDescriptorPool CreatePool(uint32_t count, VkDescriptorPoolCreateFlags flags);

    private:
        VkDevice vkDevice = VK_NULL_HANDLE;

        VkDescriptorPool currentPool = VK_NULL_HANDLE;
        PoolSizes descriptorPoolSize{};
        std::vector<VkDescriptorPool> usedPools{};
        std::vector<VkDescriptorPool> freePools{};
    };

    class DescriptorSetLayoutBuilder {

    };
}