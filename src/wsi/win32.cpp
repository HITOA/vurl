#include <vurl/wsi/win32.cpp>


VkSurfaceKHR Vurl::WSIWin32::CreateSurface(VkInstance vkInstance) {
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hwnd = hwnd;
    surfaceCreateInfo.hinstance = hinstance;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &vkSurface);

    return vkSurface;
}

void Vurl::WSIWin32::DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) {
    vkDestroySurfaceKHR(vkInstance, surface, nullptr);
}
