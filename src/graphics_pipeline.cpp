#include <vurl/graphics_pipeline.hpp>


bool Vurl::GraphicsPipeline::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS)
        return false;

    return true;
}

void Vurl::GraphicsPipeline::DestroyPipelineLayout() {
    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
}