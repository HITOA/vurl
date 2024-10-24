#pragma once

#include <volk.h>
#include <vurl/wsi/wsi.hpp>
#include <vurl/texture.hpp>
#include <vurl/error.hpp>
#include <memory>

namespace Vurl {
    class Surface {
    public:
        Surface() = delete;
        Surface(std::shared_ptr<WSI> wsi) : wsi{ wsi } {};
        ~Surface() = default;

        VurlResult CreateSurface(VkInstance instance);
        void DestroySurface();

        VurlResult CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height);
        void DestroySwapchain();

        inline VkSurfaceKHR GetSurfaceKHR() const { return vkSurface; }
        inline VkSwapchainKHR GetSwapchainKHR() const { return vkSwapchain; }
        inline std::shared_ptr<Resource<Texture>> GetBackBuffer() const { return renderTexture; }

        inline void SetVSync(bool enable) { vsync = enable; }
        inline bool IsVSyncEnable() const { return vsync; }

        inline void SetRelaxed(bool enable) { relaxed = enable; }
        inline bool IsVSyncRelaxed() const { return relaxed; }

        inline uint32_t GetWidth() const { return width; }
        inline uint32_t GetHeight() const { return height; }

    private:
        VkSurfaceFormatKHR SelectSwapSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t formatCount);
        VkPresentModeKHR SelectSwapPresentMode(VkPresentModeKHR* presentModes, uint32_t presentModeCount);
        VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

    private:
        std::shared_ptr<WSI> wsi = nullptr;
        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkDevice vkDevice = VK_NULL_HANDLE;

        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
        VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
        uint32_t swapchainImageCount = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        VkFormat format = VK_FORMAT_UNDEFINED;
        bool vsync = true;
        bool relaxed = true; //If vsync is false this doesn't matter

        std::shared_ptr<Resource<Texture>> renderTexture = nullptr;
    };
}