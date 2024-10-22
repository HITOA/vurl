#pragma once

#include <vurl/vulkan_header.hpp>
#include <vurl/resource.hpp>

namespace Vurl {
    struct Buffer {
        VkBuffer vkBuffer = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = 0;
        VmaAllocation allocation = VK_NULL_HANDLE;
    };
}