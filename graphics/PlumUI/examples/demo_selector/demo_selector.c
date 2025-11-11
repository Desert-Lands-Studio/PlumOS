#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "core/plum.h"
#include "render/loader.h"  // Добавляем этот include для plum_rhi_parse_args
#include "widgets/application/plumapplication.h"
#include "widgets/windows/plumwindow.h"
#include "widgets/controls/buttons/plumbutton.h"
#include "widgets/controls/displays/plumlabel.h"
#include "widgets/layouts/vboxlayout/plumvboxlayout.h"
#include "widgets/layouts/hboxlayout/plumhboxlayout.h"

typedef struct {
    PlumWindow* window;
    PlumLabel* fps_label;
    PlumLabel* info_label;
    bool running;
    float rotation;
    double last_time;
    int frame_count;
} DemoState;

typedef struct {
    float x, y, z;
} Vertex3D;

static Vertex3D cube_vertices[] = {
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}
};

static int cube_indices[] = {
    0,1, 1,2, 2,3, 3,0, 
    4,5, 5,6, 6,7, 7,4, 
    0,4, 1,5, 2,6, 3,7  
};

static void on_start_test(PlumButton* button) {
    DemoState* state = (DemoState*)button->base.userdata;
    if (state) {
        state->running = true;
        printf("Starting 3D cube test...\n");
    }
}

static void update_fps(DemoState* state) {
    double current_time = (double)clock() / CLOCKS_PER_SEC;
    state->frame_count++;
    
    if (current_time - state->last_time >= 1.0) {
        char fps_text[64];
        snprintf(fps_text, sizeof(fps_text), "FPS: %d", state->frame_count);
        
        if (state->fps_label) {
            plum_label_set_text(state->fps_label, fps_text);
        }
        
        state->frame_count = 0;
        state->last_time = current_time;
    }
}

static void render_cube_frame(DemoState* state) {
    if (!state || !state->running) return;
    
    state->rotation += 0.02f;
    if (state->rotation > 2 * M_PI) {
        state->rotation -= 2 * M_PI;
    }
    
    printf("Rendering cube - Rotation: %.2f radians\n", state->rotation);
    
    update_fps(state);
}

static void demo_main_loop(PlumWindow* window) {
    DemoState* state = (DemoState*)window->base.userdata;
    if (!state) return;
    
    state->running = true;
    state->last_time = (double)clock() / CLOCKS_PER_SEC;
    
    for (int i = 0; i < 300 && state->running; i++) { 
        render_cube_frame(state);
        usleep(16000); 
    }
    
    state->running = false;
    printf("Demo completed!\n");
}

int main(int argc, char** argv) {  // Исправленная сигнатура main
    printf("=== PlumUI Demo Selector ===\n\n");
    
    plum_rhi_parse_args(argc, argv);
    plum_rhi_parse_env();
    
    if (!plum_init()) {
        fprintf(stderr, "Failed to initialize PlumUI\n");
        return 1;
    }
    
    printf("✅ PlumUI initialized successfully\n");
    
    PlumWindow* window = plum_window_create("PlumUI Demo Selector", 600, 400);
    if (!window) {
        printf("❌ Failed to create window\n");
        plum_quit();
        return -1;
    }
    
    DemoState* state = (DemoState*)malloc(sizeof(DemoState));
    memset(state, 0, sizeof(DemoState));
    state->window = window;
    window->base.userdata = state;
    
    PlumLabel* title_label = plum_label_new("PlumUI Demo Selector");
    PlumLabel* info_label = plum_label_new("Select Rendering Backend:");
    
    PlumButton* start_button = plum_button_new("Start 3D Cube Test");
    state->fps_label = plum_label_new("FPS: 0");
    state->info_label = plum_label_new("Ready to start demo");
    
    plum_button_set_on_click(start_button, on_start_test);
    start_button->base.userdata = state;
    
    printf("✅ Demo UI created successfully\n");
    printf("Starting demo...\n");
    
    demo_main_loop(window);
    
    printf("Cleaning up...\n");
    plum_window_destroy(window);
    free(state);
    plum_quit();
    
    printf("🎉 Demo completed successfully!\n");
    return 0;
}