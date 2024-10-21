#pragma once

#include <volk.h>
#include <vurl/resource.hpp>

namespace Vurl {
    enum class TextureSizeClass {
        Absolute,
        SwapchainRelative
    };

    struct Texture {
        VkImage vkImage = VK_NULL_HANDLE;
        VkImageView vkImageView = VK_NULL_HANDLE;
        VkFormat vkFormat = VK_FORMAT_UNDEFINED;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureSizeClass sizeClass = TextureSizeClass::Absolute;
    };

    /*class RenderTexture : public Resource<Texture> {
        
    };

    class Texture2D : public Resource<Texture> {

    };

    class Cubemap : public Resource<Texture> {

    };*/
}