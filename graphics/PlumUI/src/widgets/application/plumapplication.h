#pragma once
#include <stdbool.h>
#include "core/plumlist.h"
#include "render/rhi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PlumApplication PlumApplication;

struct PlumApplication {
    bool is_running;
    PlumList* windows;
    PlumRhi* rhi_context;
    int argc;
    char** argv;
};

PlumApplication* plum_application_new(int argc, char** argv);
bool plum_application_run(PlumApplication* app);
void plum_application_quit(PlumApplication* app);
void plum_application_process_events(PlumApplication* app); 
void plum_application_destroy(PlumApplication* app);

#ifdef __cplusplus
}
#endif
