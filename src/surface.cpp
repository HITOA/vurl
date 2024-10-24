#include <vurl/surface.hpp>
#include <algorithm>
#include <vector>

Vurl::VurlResult Vurl::Surface::CreateSurface(VkInstance instance) {
    vkSurface = wsi->CreateSurface(instance);
    vkInstance = instance;
    return VURL_SUCCESS;
}

void Vurl::Surface::DestroySurface() {
    wsi->DestroySurface(vkInstance, vkSurface);
}

Vurl::VurlResult Vurl::Surface::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height) {
    VkSurfaceCapabilitiesKHR capabilities;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &capabilities);

    uint32_t formatCount;
    uint32_t presentModeCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &formatCount, formats.data());

    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR selectedFormat = SelectSwapSurfaceFormat(formats.data(), formatCount);
    VkPresentModeKHR selectedPresentMode = SelectSwapPresentMode(presentModes.data(), presentModeCount);
    VkExtent2D extent = SelectSwapExtent(capabilities, width, height);

    swapchainImageCount = capabilities.minImageCount + 1;
    
    if (capabilities.maxImageCount > 0 && swapchainImageCount > capabilities.maxImageCount)
        swapchainImageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vkSurface;
    swapchainCreateInfo.minImageCount = swapchainImageCount;
    swapchainCreateInfo.imageFormat = selectedFormat.format;
    swapchainCreateInfo.imageColorSpace = selectedFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = selectedPresentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &vkSwapchain) != VK_SUCCESS)
        return VURL_ERROR_SWAPCHAIN_CREATION_FAILED;

    vkGetSwapchainImagesKHR(device, vkSwapchain, &swapchainImageCount, nullptr);
    renderTexture = std::make_shared<Resource<Texture>>("Back Buffer");
    renderTexture->SetSliceCount(swapchainImageCount);

    std::vector<VkImage> swapchainImages(swapchainImageCount);

    vkGetSwapchainImagesKHR(device, vkSwapchain, &swapchainImageCount, swapchainImages.data());

    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        texture->vkImage = swapchainImages[i];
        
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = texture->vkImage;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = selectedFormat.format;
        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &viewCreateInfo, nullptr, &texture->vkImageView);
        
        texture->vkFormat = selectedFormat.format;
        texture->width = width;
        texture->height = height;

        renderTexture->SetResourceSlice(texture, i);
    }

    vkPhysicalDevice = physicalDevice;
    vkDevice = device;

    this->width = extent.width;
    this->height = extent.height;
    this->format = selectedFormat.format;

    return VURL_SUCCESS;
}

void Vurl::Surface::DestroySwapchain() {
    for (uint32_t i = 0; i < renderTexture->GetSliceCount(); ++i)
        vkDestroyImageView(vkDevice, renderTexture->GetResourceSlice(i)->vkImageView, nullptr);

    renderTexture = nullptr;
    vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
}

VkSurfaceFormatKHR Vurl::Surface::SelectSwapSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t formatCount) {
    for (uint32_t i = 0; i < formatCount; ++i) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return formats[i];
    }

    return formats[0];
}

VkPresentModeKHR Vurl::Surface::SelectSwapPresentMode(VkPresentModeKHR* presentModes, uint32_t presentModeCount) {
    if (vsync && !relaxed)
        return VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (vsync && relaxed && presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        if (!vsync && presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vurl::Surface::SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    return VkExtent2D{
        std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };
}