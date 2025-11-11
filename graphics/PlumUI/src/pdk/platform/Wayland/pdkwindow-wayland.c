#include "pdkwindow_p.h"
#include <wayland-client.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../../../generated/xdg-shell-client-protocol.h"


static void xdg_surface_configure_handler(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

static void xdg_toplevel_configure_handler(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    
}

static void xdg_toplevel_close_handler(void *data, struct xdg_toplevel *xdg_toplevel) {
    
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
};

static struct wl_compositor* global_compositor = NULL;
static struct xdg_wm_base* global_xdg_wm_base = NULL;

static void registry_global_handler(void* data,
                                   struct wl_registry* registry,
                                   uint32_t id,
                                   const char* interface,
                                   uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        global_compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        global_xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = NULL,
};

PdkWindow* pdk_window_create(const char* title, int width, int height) {
    PdkWindow* win = calloc(1, sizeof(PdkWindow));  
    if (!win) return NULL;

    win->display = wl_display_connect(NULL);
    if (!win->display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        free(win);
        return NULL;
    }

    struct wl_registry* registry = wl_display_get_registry(win->display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(win->display);

    if (!global_compositor || !global_xdg_wm_base) {
        fprintf(stderr, "Failed to bind compositor or xdg_wm_base\n");
        wl_registry_destroy(registry);
        wl_display_disconnect(win->display);
        free(win);
        return NULL;
    }

    win->compositor = global_compositor;
    win->xdg_wm_base = global_xdg_wm_base;

    win->surface = wl_compositor_create_surface(win->compositor);
    if (!win->surface) goto fail;

    win->xdg_surface = xdg_wm_base_get_xdg_surface(win->xdg_wm_base, win->surface);
    if (!win->xdg_surface) goto fail;

    
    xdg_surface_add_listener(win->xdg_surface, &xdg_surface_listener, win);

    win->xdg_toplevel = xdg_surface_get_toplevel(win->xdg_surface);
    if (!win->xdg_toplevel) goto fail;

    
    xdg_toplevel_add_listener(win->xdg_toplevel, &xdg_toplevel_listener, win);

    if (title) {
        xdg_toplevel_set_title(win->xdg_toplevel, title);
        win->title = strdup(title);
    }

    win->width = width;
    win->height = height;

    
    xdg_surface_set_window_geometry(win->xdg_surface, 0, 0, width, height);

    wl_surface_commit(win->surface);
    wl_display_roundtrip(win->display);

    wl_registry_destroy(registry);
    printf("✅ Wayland window created: %s (%dx%d)\n", title, width, height);
    return win;

fail:
    pdk_window_destroy(win);
    wl_registry_destroy(registry);
    return NULL;
}

void pdk_window_destroy(PdkWindow* win) {
    if (!win) return;

    if (win->xdg_toplevel) xdg_toplevel_destroy(win->xdg_toplevel);
    if (win->xdg_surface) xdg_surface_destroy(win->xdg_surface);
    if (win->surface) wl_surface_destroy(win->surface);
    if (win->title) free(win->title);

    
    
    

    wl_display_disconnect(win->display);
    free(win);
}