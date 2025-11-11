#ifndef PDK_WINDOW_WAYLAND_H
#define PDK_WINDOW_WAYLAND_H

#include "../../pdkwindow.h"
#include <wayland-client.h>
#include "../../../../generated/xdg-shell-client-protocol.h"

struct PdkWindow {
    struct wl_display* display;
    struct wl_surface* surface;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct wl_compositor* compositor;
    struct xdg_wm_base* xdg_wm_base;
    int width;
    int height;
    char* title;
};

#endif