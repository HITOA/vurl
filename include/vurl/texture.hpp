#pragma once

#include <vurl/vulkan_header.hpp>
#include <vurl/resource.hpp>

namespace Vurl {
    enum class TextureSizeClass {
        Absolute,
        SwapchainRelative
    };

    struct Texture {
        VkImage vkImage = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView vkImageView = VK_NULL_HANDLE;
        VkFormat vkFormat = VK_FORMAT_UNDEFINED;
        VkImageType vkImageType = VK_IMAGE_TYPE_2D;
        VkImageViewType vkImageViewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageTiling vkImageTiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags usage = 0;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        TextureSizeClass sizeClass = TextureSizeClass::Absolute;
    };
}