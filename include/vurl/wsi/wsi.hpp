#pragma once

#include <vurl/vulkan_header.hpp>

namespace Vurl {

    //window system integration base class
    class WSI {
    public:
        virtual VkSurfaceKHR CreateSurface(VkInstance vkInstance) = 0;
        virtual void DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) = 0;
    };
    
}