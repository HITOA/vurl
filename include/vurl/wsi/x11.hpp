#pragma once

#include <vurl/wsi/wsi.hpp>
//Missing x11 include

namespace Vurl {

    class WSIX11 : public WSI {
    public:
        virtual VkSurfaceKHR CreateSurface(VkInstance vkInstance) override;
        virtual void DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) override;

        inline void SetDPY(Display* dpy) { this->dpy = dpy; }
        inline void SetWindow(Window* window) { this->window = window; }
        inline Display* GetDPY() const { return dpy; }
        inline Window GetWindow() const { return window; }

    private:
        Display* dpy;
        Window window;
    };

}