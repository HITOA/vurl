#pragma once

namespace Vurl {
    enum VurlResult {
        VURL_SUCCESS = 0,
        VURL_ERROR_UNSUPPORTED_INSTANCE_VERSION = 1,
        VURL_ERROR_UNSUPPORTED_INSTANCE_EXTENSION = 2,
        VURL_ERROR_INSTANCE_CREATION_FAILED = 3,
        VURL_ERROR_MISSING_INSTANCE = 4,
        VURL_ERROR_NO_PHYSICAL_DEVICE = 5,
        VURL_ERROR_DEVICE_CREATION_FAILED = 6,
        VURL_ERROR_SWAPCHAIN_CREATION_FAILED = 10
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
            "Swapchain creation failed."
        };
        return messages[result];
    }
}