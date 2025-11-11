#include "pdkdisplay.h"
#include "core/plumallocator.h"
#include <wayland-client.h>
#include <stdio.h>

PdkDisplay* pdk_display_default(void) {
    static struct wl_display* wl_display = NULL;
    
    if (!wl_display) {
        wl_display = wl_display_connect(NULL);
        if (!wl_display) {
            fprintf(stderr, "Failed to connect to Wayland display\n");
            return NULL;
        }
        printf("✅ Wayland display initialized\n");
    }
    
    return (PdkDisplay*)wl_display;
}

void pdk_display_close(PdkDisplay* display) {
    if (display) {
        wl_display_disconnect((struct wl_display*)display);
    }
}