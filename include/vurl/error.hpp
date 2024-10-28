#pragma once

namespace Vurl {
    enum VurlResult {
        VURL_SUCCESS = 0,
        VURL_ERROR_UNSUPPORTED_INSTANCE_VERSION,
        VURL_ERROR_UNSUPPORTED_INSTANCE_EXTENSION,
        VURL_ERROR_INSTANCE_CREATION_FAILED,
        VURL_ERROR_MISSING_INSTANCE,
        VURL_ERROR_NO_PHYSICAL_DEVICE,
        VURL_ERROR_DEVICE_CREATION_FAILED,
        VURL_ERROR_SWAPCHAIN_CREATION_FAILED,
        VURL_ERROR_SHADER_MODULE_CREATION_FAILED,
        VURL_ERROR_REFLECT_SHADER_MODULE_CREATION_FAILD
    };

    inline const char* GetErrorMessage(VurlResult result) {
        const char* messages[] = {
            "Success.",
            "Unsupported vulkan instance version.",
            "Unsupported vulkan instance extension."
            "Vulkan instance creation failed.",
            "Vulkan instance is missing. Create a vulkan instance before any other operation.",
            "No suitable physical device found.",
            "Logical device creation failed.",
            "Swapchain creation failed.",
            "Shader module creation failed.",
            "Reflection shader module creation failed."
        };
        return messages[result];
    }
}