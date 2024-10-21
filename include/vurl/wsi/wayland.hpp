#pragma once

#include <vurl/wsi/wsi.hpp>
#include <wayland-client.h>

namespace Vurl {

    class WSIWayland : public WSI {
    public:
        virtual VkSurfaceKHR CreateSurface(VkInstance vkInstance) override;
        virtual void DestroySurface(VkInstance vkInstance, VkSurfaceKHR surface) override;

        inline void SetWaylandDisplay(wl_display* display) { wlDisplay = display; }
        inline void SetWaylandSurface(wl_surface* surface) { wlSurface = surface; }
        inline wl_display* GetWaylandDisplay() const { return wlDisplay; }
        inline wl_surface* GetWaylandSurface() const { return wlSurface; }

    private:
        wl_display* wlDisplay = nullptr;
        wl_surface* wlSurface = nullptr;
    };

}