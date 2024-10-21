#pragma once

#include <vurl/wsi/wsi.hpp>
//Missing win32 include

namespace Vurl {

    class WSIWin32 : public WSI {
    public:
        virtual VkSurfaceKHR CreateSurface(VkInstance vkInstance) override;
        virtual void DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) override;

        inline void SetHWND(HWND hwnd) { this->hwnd = hwnd; }
        inline void SetHINSTANCE(HINSTANCE hinstance) { this->hinstance = hinstance; }
        inline HWND GetHWND() const { return hwnd; }
        inline HINSTANCE GetHINSTANCE() const { return hinstance; }

    private:
        HWND hwnd;
        HINSTANCE hinstance;
    };

}