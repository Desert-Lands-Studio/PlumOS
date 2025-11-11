#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GLContext GLContext;

typedef struct {
    bool (*init)(void* native_window, int width, int height);
    void (*make_current)(void);
    void (*swap_buffers)(void);
    void (*destroy)(void);  
    void* (*get_proc_address)(const char* name);
} GLContextInterface;

GLContext* glcontext_create_for_platform(void* native_window, int width, int height);
void glcontext_destroy(void);  
bool glcontext_is_initialized(void);
void glcontext_set_current_context(GLContext* ctx);

#ifdef __cplusplus
}
#endif
