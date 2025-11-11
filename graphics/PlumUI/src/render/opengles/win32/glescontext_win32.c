#include "glcontext.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <stdlib.h>
#include <stdio.h>

struct GLContext {
    CGLContextObj context;
    CGLPixelFormatObj pixel_format;
};

static GLContext* g_current_context = NULL;

static bool cgl_init(void* native_window, int width, int height) {
    
    
    (void)native_window;
    (void)width;
    (void)height;

    GLContext* ctx = (GLContext*)calloc(1, sizeof(GLContext));
    if (!ctx) return false;

    
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
        kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
        kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
        kCGLPFAStencilSize, (CGLPixelFormatAttribute)8,
        kCGLPFAAccelerated,
        kCGLPFADoubleBuffer,
        (CGLPixelFormatAttribute)0
    };

    GLint num_pixel_formats;
    CGLError error = CGLChoosePixelFormat(attribs, &ctx->pixel_format, &num_pixel_formats);
    if (error != kCGLNoError || !ctx->pixel_format) {
        fprintf(stderr, "CGL: Failed to choose pixel format: %s\n", CGLErrorString(error));
        free(ctx);
        return false;
    }

    error = CGLCreateContext(ctx->pixel_format, NULL, &ctx->context);
    if (error != kCGLNoError || !ctx->context) {
        fprintf(stderr, "CGL: Failed to create context: %s\n", CGLErrorString(error));
        CGLDestroyPixelFormat(ctx->pixel_format);
        free(ctx);
        return false;
    }

    error = CGLSetCurrentContext(ctx->context);
    if (error != kCGLNoError) {
        fprintf(stderr, "CGL: Failed to make context current: %s\n", CGLErrorString(error));
        CGLDestroyContext(ctx->context);
        CGLDestroyPixelFormat(ctx->pixel_format);
        free(ctx);
        return false;
    }

    g_current_context = ctx;
    printf("✅ CGL Context initialized successfully\n");
    return true;
}

static void cgl_make_current(void) {
    if (!g_current_context) return;
    CGLSetCurrentContext(g_current_context->context);
}

static void cgl_swap_buffers(void) {
    if (!g_current_context) return;
    CGLFlushDrawable(g_current_context->context);
}

static void cgl_destroy(void) {
    if (!g_current_context) return;

    CGLSetCurrentContext(NULL);
    CGLDestroyContext(g_current_context->context);
    CGLDestroyPixelFormat(g_current_context->pixel_format);

    free(g_current_context);
    g_current_context = NULL;
    printf("✅ CGL Context destroyed\n");
}

static void* cgl_get_proc_address(const char* name) {
    
    
    if (g_current_context) {
        return (void*)CGLGetProcAddress(g_current_context->context, (const GLubyte*)name);
    }
    return NULL;
}

GLContextInterface g_cgl_interface = {
    .init = cgl_init,
    .make_current = cgl_make_current,
    .swap_buffers = cgl_swap_buffers,
    .destroy = cgl_destroy,
    .get_proc_address = cgl_get_proc_address
};

GLContext* glcontext_create_for_platform(void* native_window, int width, int height) {
    if (!g_cgl_interface.init(native_window, width, height)) {
        return NULL;
    }
    return g_current_context;
}

void glcontext_destroy(void) {
    if (g_current_context) {
        g_current_context->destroy();
    }
}

bool glcontext_is_initialized(void) {
    return g_current_context != NULL;
}

void glcontext_set_current_context(GLContext* ctx) {
    g_current_context = ctx;
}
