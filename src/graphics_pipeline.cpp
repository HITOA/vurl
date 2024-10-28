#include <vurl/graphics_pipeline.hpp>


bool Vurl::GraphicsPipeline::CreatePipelineLayout() {
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = (uint32_t)layoutBindings.size();
    layoutCreateInfo.pBindings = layoutBindings.data();

    vkCreateDescriptorSetLayout(vkDevice, &layoutCreateInfo, nullptr, &vkDescriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vkDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
        return false;

    return true;
}

void Vurl::GraphicsPipeline::DestroyPipelineLayout() {
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
}