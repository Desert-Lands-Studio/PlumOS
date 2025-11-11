/* ── PlumX/PlumUI/src/widgets/plumwindow.c ─────────────────────────────────────────── */
#include "plumwindow.h"
#include "core/plumallocator.h"
#include "pdk/pdkwindow.h"
#include "psk/pskrenderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <unistd.h> 



static void pdk_event_to_plum_event(PdkWindow* pdk_win, void* pdk_event, PlumEvent* out_event) {
    
    (void)pdk_win;
    (void)pdk_event;
    memset(out_event, 0, sizeof(PlumEvent));
    out_event->type = PLUM_EVENT_NONE;
}


static void plum_window_main_loop(PlumWindow* window) {
    if (!window) return;

    window->is_running = true;

    while (window->is_running) {
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        printf("Rendering frame...\n");
        sleep(1); 

        
        if (window->renderer) {
            psk_renderer_begin_frame(window->renderer);

            
            if (window->base.children && plum_list_size(window->base.children) > 0) {
                PlumWidget* root = (PlumWidget*)plum_list_get(window->base.children, 0);
                if (root && root->paint) {
                    root->paint(root, window->renderer);
                }
            }

            psk_renderer_end_frame(window->renderer);
        }
    }
}


PLUM_API PlumWindow* plum_window_create(const char* title, int width, int height) {
    PlumWindow* win = (PlumWindow*)plum_alloc(sizeof(PlumWindow));
    if (!win) return NULL;

    
    memset(&win->base, 0, sizeof(PlumWidget));
    win->base.x = 0;
    win->base.y = 0;
    win->base.width = width;
    win->base.height = height;
    win->base.visible = true;
    win->base.children = plum_list_new(); 

    
    win->pdk_window = pdk_window_create(title, width, height);
    if (!win->pdk_window) {
        plum_free(win);
        return NULL;
    }

    win->renderer = psk_renderer_new(plum_get_rhi_context(), win->pdk_window);
    if (!win->renderer) {
        
        (win->pdk_window);
        plum_free(win);
        return NULL;
    }

    
    win->on_event = NULL;
    win->is_running = false;
    win->is_fullscreen = false;

    return win;
}


PLUM_API void plum_window_destroy(PlumWindow* window) {
    if (!window) return;

    
    plum_widget_destroy((PlumWidget*)window);

    
    if (window->renderer) {
        psk_renderer_free(window->renderer);
    }
    if (window->pdk_window) {
        pdk_window_destroy(window->pdk_window);
    }

    plum_free(window);
}


PLUM_API void plum_window_set_root_widget(PlumWindow* window, PlumWidget* root) {
    if (!window || !root) return;

    
    if (window->base.children && plum_list_size(window->base.children) > 0) {
        PlumWidget* old_root = (PlumWidget*)plum_list_get(window->base.children, 0);
        plum_widget_remove_child((PlumWidget*)window, old_root);
    }

    
    plum_widget_add_child((PlumWidget*)window, root);
}


PLUM_API void plum_window_run(PlumWindow* window) {
    if (!window) return;
    plum_window_main_loop(window);
}