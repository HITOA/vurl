#include <vurl/wsi/x11.hpp>


VkSurfaceKHR Vurl::WSIX11::CreateSurface(VkInstance vkInstance) {
    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.dpy = dpy;
    surfaceCreateInfo.window = window;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    vkCreateXlibSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &vkSurface);

    return vkSurface;
}

void Vurl::WSIX11::DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) {
    vkDestroySurfaceKHR(vkInstance, surface, nullptr);
}
