#include <vurl/graphics_pipeline.hpp>


bool Vurl::GraphicsPipeline::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
        return false;

    return true;
}

void Vurl::GraphicsPipeline::DestroyPipelineLayout() {
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
}