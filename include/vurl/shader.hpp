#pragma once

#include <vurl/vulkan_header.hpp>
#include <vurl/error.hpp>
#include <string>

namespace Vurl {
    class Shader {
    public:
        Shader() = delete;
        Shader(VkDevice device);
        ~Shader();

        VurlResult CreateShaderModule(const uint32_t* source, uint32_t size);
        void DestroyShaderModule();
        inline VkShaderModule GetShaderModule() const { return vkShaderModule; }
        inline SpvReflectShaderModule GetReflectShaderModule() const { return spvReflectShaderModule; }
        
        inline void SetEntryPointName(const std::string& name) { entrypointName = name; }
        inline const char* GetEntryPointName() const { return entrypointName.c_str(); }

    private:
        VkDevice vkDevice = VK_NULL_HANDLE;

        VkShaderModule vkShaderModule = VK_NULL_HANDLE;
        SpvReflectShaderModule spvReflectShaderModule = {};
        std::string entrypointName = "main";
    };
}