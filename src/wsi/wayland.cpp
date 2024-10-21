#include <vurl/wsi/wayland.hpp>


VkSurfaceKHR Vurl::WSIWayland::CreateSurface(VkInstance vkInstance) {
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.display = wlDisplay;
    surfaceCreateInfo.surface = wlSurface;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    vkCreateWaylandSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &vkSurface);
    
    return vkSurface;
}

void Vurl::WSIWayland::DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) {
    vkDestroySurfaceKHR(vkInstance, surface, nullptr);
}