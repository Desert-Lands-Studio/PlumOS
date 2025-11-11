#include "pdkdisplay.h"
#include "core/plumallocator.h"
#include <X11/Xlib.h>
#include <stdio.h>

PdkDisplay* pdk_display_default(void) {
    static Display* xdisplay = NULL;
    
    if (!xdisplay) {
        xdisplay = XOpenDisplay(NULL);
        if (!xdisplay) {
            fprintf(stderr, "Failed to open X11 display\n");
            return NULL;
        }
        printf("✅ X11 display initialized\n");
    }
    
    
    return (PdkDisplay*)xdisplay;
}


void pdk_display_close(PdkDisplay* display) {
    if (display) {
        XCloseDisplay((Display*)display);
    }
}
