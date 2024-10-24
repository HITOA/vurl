#include <vurl/rendering_context.hpp>

#include <cstring>
#include <cstdlib>
#include <vector>
#include <iostream>

Vurl::VurlResult Vurl::RenderingContext::CreateInstance(const VkApplicationInfo* applicationInfo, const char** instanceExtensions, 
        uint32_t instanceExtensionCount, bool enableValidationLayer) {
    if (applicationInfo && volkGetInstanceVersion() < applicationInfo->apiVersion)
        return VURL_ERROR_UNSUPPORTED_INSTANCE_VERSION;
    
    VkApplicationInfo defaultApplicationInfo{};
    defaultApplicationInfo.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO;
    defaultApplicationInfo.pApplicationName = "null";
    defaultApplicationInfo.applicationVersion = 0;
    defaultApplicationInfo.pEngineName = "vurl";
    defaultApplicationInfo.engineVersion = 0;
    defaultApplicationInfo.apiVersion = volkGetInstanceVersion();
    
    if (!applicationInfo)
        applicationInfo = &defaultApplicationInfo;

    vulkanApiVersion = applicationInfo->apiVersion;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = applicationInfo;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
        if (!HasExtension(extensions.data(), extensionCount, instanceExtensions[i]))
            return VURL_ERROR_UNSUPPORTED_INSTANCE_EXTENSION;

    enabledInstanceExtensions.resize(instanceExtensionCount);
    for (uint32_t i = 0; i < instanceExtensionCount; ++i)
        enabledInstanceExtensions[i] = instanceExtensions[i];

    createInfo.enabledExtensionCount = (uint32_t)enabledInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledInstanceExtensions.data();

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    enabledValidationLayers.clear();

    if (enableValidationLayer) {
        if (HasLayer(layers.data(), layerCount, "VK_LAYER_KHRONOS_validation")) {
            enabledValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
        } else {
            std::cout << "Missing validation layer." << std::endl;
        }
    }
    
    createInfo.enabledLayerCount = (uint32_t)enabledValidationLayers.size();
    createInfo.ppEnabledLayerNames = enabledValidationLayers.data();

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
        return VURL_ERROR_INSTANCE_CREATION_FAILED;

    volkLoadInstance(vkInstance);

    return VURL_SUCCESS;
}

void Vurl::RenderingContext::DestroyInstance() {
    if (vkDevice != VK_NULL_HANDLE)
        DestroyDevice();
    
    vkDestroyInstance(vkInstance, nullptr);
    vkInstance = VK_NULL_HANDLE;
}

Vurl::VurlResult Vurl::RenderingContext::CreateDevice(VkSurfaceKHR surface, VkPhysicalDevice device) {
    if (vkInstance == VK_NULL_HANDLE)
        return VURL_ERROR_MISSING_INSTANCE;
    
    std::vector<const char*> requiredDeviceExtensions{};
    requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    requiredDeviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    requiredDeviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

    VkPhysicalDevice selectedDevice = device;

    if (selectedDevice == VK_NULL_HANDLE) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
            return VURL_ERROR_NO_PHYSICAL_DEVICE;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
        
        int selectedDeviceScore = -1;

        for (auto device : devices) {
            int currentDeviceScore = RatePhysicalDevice(surface, device, requiredDeviceExtensions.data(), (uint32_t)requiredDeviceExtensions.size());
            if (selectedDeviceScore < currentDeviceScore) {
                selectedDevice = device;
                selectedDeviceScore = currentDeviceScore;
            }
        }

        if (selectedDevice == VK_NULL_HANDLE)
            return VURL_ERROR_NO_PHYSICAL_DEVICE; //Could not find suitable physical device.
    } else {
        if (RatePhysicalDevice(surface, selectedDevice, requiredDeviceExtensions.data(), (uint32_t)requiredDeviceExtensions.size()) == -1)
            return VURL_ERROR_NO_PHYSICAL_DEVICE; //Given physical device is not suitable.
    }

    enabledDeviceExtensions = requiredDeviceExtensions;

    QueueInfo selectedDeviceQueueinfo = GetPhysicalDeviceQueueInfo(surface, selectedDevice);

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = selectedDeviceQueueinfo.familyIndices[QUEUE_INDEX_GRAPHICS];
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledLayerCount = (uint32_t)enabledValidationLayers.size();
    createInfo.ppEnabledLayerNames = enabledValidationLayers.data();
    createInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
    createInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data(); 

    if (vkCreateDevice(selectedDevice, &createInfo, nullptr, &vkDevice) != VK_SUCCESS)
        return VURL_ERROR_DEVICE_CREATION_FAILED; //Unable to create logical device.

    vkPhysicalDevice = selectedDevice;
    queueInfo = selectedDeviceQueueinfo;

    vkGetDeviceQueue(vkDevice, queueInfo.familyIndices[QUEUE_INDEX_GRAPHICS], 0, &queueInfo.queues[QUEUE_INDEX_GRAPHICS]);

    volkLoadDevice(vkDevice);

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    if (vulkanApiVersion <= VK_API_VERSION_1_0) {
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
        vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
        vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    } else {
        vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
        vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
        vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
        vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    }
    vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.flags = 0;
    allocatorCreateInfo.vulkanApiVersion = vulkanApiVersion;
    allocatorCreateInfo.physicalDevice = vkPhysicalDevice;
    allocatorCreateInfo.device = vkDevice;
    allocatorCreateInfo.instance = vkInstance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);

    return VURL_SUCCESS;
}

void Vurl::RenderingContext::DestroyDevice() {
    vmaDestroyAllocator(vmaAllocator);
    vkDeviceWaitIdle(vkDevice);
    vkDestroyDevice(vkDevice, nullptr);
    vkDevice = VK_NULL_HANDLE;
    vkPhysicalDevice = VK_NULL_HANDLE;
}

bool Vurl::RenderingContext::HasExtension(VkExtensionProperties* extensions, uint32_t extensionCount, const char* extension) {
    VkExtensionProperties* end = extensions + extensionCount;
    for (VkExtensionProperties* it = extensions; it != end; ++it)
        if (strcmp(it->extensionName, extension) == 0)
            return true;
    return false;
}

bool Vurl::RenderingContext::HasLayer(VkLayerProperties* layers, uint32_t layerCount, const char* layer) {
    VkLayerProperties* end = layers + layerCount;
    for (VkLayerProperties* it = layers; it != end; ++it)
        if (strcmp(it->layerName, layer) == 0)
            return true;
    return false;
}

Vurl::QueueInfo Vurl::RenderingContext::GetPhysicalDeviceQueueInfo(VkSurfaceKHR surface, VkPhysicalDevice device) {
    QueueInfo queueInfo{};
    memset(&queueInfo, 0x0, sizeof(QueueInfo));

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueInfo.counts[QUEUE_INDEX_GRAPHICS] == 0) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                queueInfo.familyIndices[QUEUE_INDEX_GRAPHICS] = i;
                queueInfo.counts[QUEUE_INDEX_GRAPHICS] = queueFamilies[i].queueCount;
            }
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && queueInfo.counts[QUEUE_INDEX_COMPUTE] == 0) {
            queueInfo.familyIndices[QUEUE_INDEX_COMPUTE] = i;
            queueInfo.counts[QUEUE_INDEX_COMPUTE] = queueFamilies[i].queueCount;
        }

        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && queueInfo.counts[QUEUE_INDEX_TRANSFER] == 0) {
            queueInfo.familyIndices[QUEUE_INDEX_TRANSFER] = i;
            queueInfo.counts[QUEUE_INDEX_TRANSFER] = queueFamilies[i].queueCount;
        }
    }

    return queueInfo;
}

int Vurl::RenderingContext::RatePhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice device, const char** extensions, uint32_t extensionCount) {
    int score = 0;

    QueueInfo queueInfo = GetPhysicalDeviceQueueInfo(surface, device);
    if (queueInfo.counts[QUEUE_INDEX_GRAPHICS] == 0)
        return -1; //Device not suitable, graphics queue missing

    uint32_t availableExtensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (!HasExtension(availableExtensions.data(), availableExtensionCount, extensions[i])) {
            return -1;
        }
    }


    uint32_t formatCount;
    uint32_t presentModeCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());

    if (formats.empty())
        return -1;

    score += formats.size();

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());

    if (presentModes.empty())
        return -1;

    score += presentModes.size();

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 100;

    return score;
}