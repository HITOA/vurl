#pragma once

#include <vurl/vulkan_header.hpp>
#include <vurl/vulkan_def.hpp>
#include <vector>

namespace Vurl {

    class RenderingContext {
    public:
        RenderingContext() = default;
        ~RenderingContext() = default;

        bool CreateInstance(const VkApplicationInfo* applicationInfo, const char** instanceExtensions, 
                uint32_t instanceExtensionCount, bool enableValidationLayer = false);
        void DestroyInstance();

        bool CreateDevice(VkSurfaceKHR surface, VkPhysicalDevice device = VK_NULL_HANDLE);
        void DestroyDevice();

        inline VkInstance GetInstance() const { return vkInstance; }
        inline VkPhysicalDevice GetPhysicalDevice() const { return vkPhysicalDevice; }
        inline VkDevice GetDevice() const { return vkDevice; }
        inline const QueueInfo& GetQueueInfo() const { return queueInfo; }

    private:
        bool HasExtension(VkExtensionProperties* extensions, uint32_t extensionCount, const char* extension);
        bool HasLayer(VkLayerProperties* layers, uint32_t layerCount, const char* layer);
        QueueInfo GetPhysicalDeviceQueueInfo(VkSurfaceKHR surface, VkPhysicalDevice device);
        int RatePhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice device, const char** extensions, uint32_t extensionCount);

    private:
        VkInstance vkInstance = VK_NULL_HANDLE;
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
        VkDevice vkDevice = VK_NULL_HANDLE;
        VmaAllocator vmaAllocator = VK_NULL_HANDLE;
        QueueInfo queueInfo;
        uint32_t vulkanApiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> enabledValidationLayers{};
        std::vector<const char*> enabledInstanceExtensions{};
        std::vector<const char*> enabledDeviceExtensions{};
    };

}