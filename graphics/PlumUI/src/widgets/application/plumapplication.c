#include "plumapplication.h"
#include "core/plum.h"
#include "render/rhi.h"
#include "render/loader.h"
#include "psk/psk.h"
#include "windows/plumwindow.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

bool plum_application_run(PlumApplication* app) {
    if (!app) return false;
    
    
    plum_rhi_parse_env();
    plum_rhi_parse_args(app->argc, app->argv); 

    if (!plum_init()) {
        return false;
    }

    
    app->rhi_context = plum_get_rhi_context();
    printf("Running application with %s\n", app->rhi_context->info->name);
    
    
    PlumWindow* window = plum_window_create("PlumUI Demo", 800, 600);
    if (!window) {
        plum_quit();
        return false;
    }
    
    
    if (!app->windows) {
        app->windows = plum_list_new();
    }
    plum_list_append(app->windows, window);
    
    
    app->is_running = true;
    while (app->is_running) {
        
        plum_application_process_events(app);
        
        
        psk_renderer_begin_frame(window->renderer);
        
        psk_renderer_end_frame(window->renderer);
    }
    
    plum_window_destroy(window);
    plum_quit();
    return true;
}


PlumApplication* plum_application_new(int argc, char** argv) {
    (void)argc;
    (void)argv;
    PlumApplication* app = (PlumApplication*)malloc(sizeof(PlumApplication));
    if (app) {
        app->is_running = false;
        app->windows = NULL;
        app->rhi_context = NULL;
        app->argc = argc;
        app->argv = argv;
    }
    return app;
}

void plum_application_quit(PlumApplication* app) {
    if (app) app->is_running = false;
}

void plum_application_process_events(PlumApplication* app) {
    
    (void)app; 
}

void plum_application_destroy(PlumApplication* app) {
    if (app) {
        if (app->windows) {
            
            plum_list_free(app->windows);
        }
        free(app);
    }
}
