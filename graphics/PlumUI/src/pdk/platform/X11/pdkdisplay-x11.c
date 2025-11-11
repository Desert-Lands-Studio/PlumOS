#include "pdkdisplay.h"
#include "core/plumallocator.h"
#include <X11/Xlib.h>
#include <stdio.h>

PdkDisplay* pdk_display_default(void) {
    static PdkDisplay* display = NULL;
    if (!display) {
        display = (PdkDisplay*)plum_alloc(sizeof(PdkDisplay));
        if (display) {
            
            Display* xdisplay = XOpenDisplay(NULL);
            if (!xdisplay) {
                plum_free(display);
                return NULL;
            }
            
            *(Display**)display = xdisplay;
            printf("✅ X11 display initialized\n");
        }
    }
    return display;
}
