#include <vurl/shader.hpp>


Vurl::Shader::Shader(VkDevice device) : vkDevice{ device } {

}

Vurl::Shader::~Shader() {

}

bool Vurl::Shader::CreateShaderModule(const uint32_t* source, uint32_t size) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = source;

    if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &vkShaderModule) != VK_SUCCESS)
        return false;

    return true;
}

void Vurl::Shader::DestroyShaderModule() {
    vkDestroyShaderModule(vkDevice, vkShaderModule, nullptr);
}