#include <vurl/shader.hpp>


Vurl::Shader::Shader(VkDevice device) : vkDevice{ device } {

}

Vurl::Shader::~Shader() {

}

Vurl::VurlResult Vurl::Shader::CreateShaderModule(const uint32_t* source, uint32_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = source;

    if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &vkShaderModule) != VK_SUCCESS)
        return VURL_ERROR_SHADER_MODULE_CREATION_FAILED;
    
    if (spvReflectCreateShaderModule(size, source, &spvReflectShaderModule) != SPV_REFLECT_RESULT_SUCCESS)
        return VURL_ERROR_REFLECT_SHADER_MODULE_CREATION_FAILD;

    return VURL_SUCCESS;
}

void Vurl::Shader::DestroyShaderModule() {
    spvReflectDestroyShaderModule(&spvReflectShaderModule);
    vkDestroyShaderModule(vkDevice, vkShaderModule, nullptr);
}