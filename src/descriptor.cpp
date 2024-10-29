#include <vurl/descriptor.hpp>


void Vurl::DescriptorSetAllocator::Reset() {
    for (uint32_t i = 0; i < usedPools.size(); ++i) {
        vkResetDescriptorPool(vkDevice, usedPools[i], 0);
        freePools.push_back(usedPools[i]);
    }

    usedPools.clear();
    currentPool = VK_NULL_HANDLE;
}

void Vurl::DescriptorSetAllocator::Destroy() {
    for (uint32_t i = 0; i < usedPools.size(); ++i)
        vkDestroyDescriptorPool(vkDevice, usedPools[i], nullptr);

    for (uint32_t i = 0; i < freePools.size(); ++i)
        vkDestroyDescriptorPool(vkDevice, freePools[i], nullptr);

    usedPools.clear();
    freePools.clear();
}

VkDescriptionSet Vurl::DescriptorSetAllocator::Allocate(VkDescriptionSetLayout layout) {
    VkDescriptionSet descriptionSet = VK_NULL_HANDLE;
    Allocate(&descriptionSet, &layout, 1);
    return descriptionSet;
}

void Vurl::DescriptorSetAllocator::Allocate(VkDescriptionSet* descriptionSets, VkDescriptionSetLayout* layouts, uint32_t count) {
    if (currentPool == VK_NULL_HANDLE) {
        currentPool = GetPool();
        usedPools.push_back(currentPool);
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = currentPool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = layouts;

    VkResult r = vkAllocateDescriptorSets(vkDevice, &allocInfo, descriptionSets);

    switch (r) {
        case VK_SUCCESS:
            return;
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            currentPool = GetPool();
            usedPools.push_back(currentPool);
            vkAllocateDescriptorSets(vkDevice, &allocInfo, descriptionSets);
            return;
        default:
            return;
    }
}

VkDescriptorPool Vurl::DescriptorSetAllocator::GetPool() {
    if (freePools.size() > 0) {
        VkDescriptorPool pool = freePools.back();
        freePools.pop_back();
        return pool;
    } else {
        return CreatePool(1000, 0);
    }
}

VkDescriptorPool Vurl::DescriptorSetAllocator::CreatePool(uint32_t count, VkDescriptorPoolCreateFlags flags) {
    std::vector<VkDescriptorPoolSize> sizes(descriptorPoolSize.sizes.size());
    for (auto size : descriptorPoolSize.sizes)
        sizes.emplace_back(size.first, (uint32_t)(size.second * count));

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags = flags;
    poolCreateInfo.maxSets = count;
    poolCreateInfo.poolSizeCount = (uint32_t)sizes.size();
    poolCreateInfo.pPoolSizes = sizes.data();

    VkDescriptorPool pool;
    vkCreateDescriptorPool(vkDevice, &poolCreateInfo, nullptr, &pool);

    return pool;
}
