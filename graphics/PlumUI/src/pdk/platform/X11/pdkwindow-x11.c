#include "pdk/pdkwindow.h"
#include "core/plumallocator.h"
#include <X11/Xlib.h>
#include <string.h>  
#include <stdlib.h>  
#include <stdio.h>


typedef struct PdkWindow {
    Display* display;
    Window window;
    int width;
    int height;
    const char* title;
} PdkWindow;

PdkWindow* pdk_window_create(const char* title, int width, int height) {
    PdkWindow* win = plum_alloc(sizeof(PdkWindow));
    if (!win) return NULL;

    win->display = XOpenDisplay(NULL);
    if (!win->display) {
        plum_free(win);
        return NULL;
    }

    win->window = XCreateSimpleWindow(win->display, DefaultRootWindow(win->display), 
                                      0, 0, width, height, 0, 0, 0);
    XStoreName(win->display, win->window, title ? title : "PlumUI Window");
    XMapWindow(win->display, win->window);
    XFlush(win->display);

    win->width = width;
    win->height = height;
    win->title = title ? strdup(title) : NULL;

    return win;
}

void pdk_window_destroy(PdkWindow* win) {
    if (!win) return;

    if (win->title) free((void*)win->title);
    XDestroyWindow(win->display, win->window);
    XCloseDisplay(win->display);
    plum_free(win);
}


void* pdk_window_poll_events(PdkWindow* win) {
    (void)win;
    
    return NULL;
}
