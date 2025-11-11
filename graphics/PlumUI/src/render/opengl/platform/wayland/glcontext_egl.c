#include "render/opengl/glcontext.h"
#include <EGL/egl.h>
#include <GL/gl.h>
#include <wayland-client.h>
#include <stdlib.h>
#include <stdio.h>
#include "pdk/platform/Wayland/pdkwindow-wayland.h"

struct GLContext {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    struct wl_surface* wl_surface;
};

static struct GLContext* g_current_context = NULL;

static bool egl_init(void* native_window, int width, int height) {
    PdkWindow* pdk_win = (PdkWindow*)native_window;
    if (!pdk_win || !pdk_win->surface) {
        fprintf(stderr, "❌ EGL: Invalid PdkWindow or surface\n");
        return false;
    }

    struct GLContext* ctx = calloc(1, sizeof(struct GLContext));
    if (!ctx) return false;

    ctx->wl_surface = pdk_win->surface;
    ctx->display = eglGetDisplay((EGLNativeDisplayType)pdk_win->display);
    if (ctx->display == EGL_NO_DISPLAY) {
        fprintf(stderr, "❌ EGL: Failed to get display\n");
        free(ctx);
        return false;
    }

    if (!eglInitialize(ctx->display, NULL, NULL)) {
        fprintf(stderr, "❌ EGL: Failed to initialize\n");
        free(ctx);
        return false;
    }

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(ctx->display, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        fprintf(stderr, "❌ EGL: Failed to choose config for OpenGL\n");
        eglTerminate(ctx->display);
        free(ctx);
        return false;
    }

    ctx->surface = eglCreateWindowSurface(ctx->display, config, (EGLNativeWindowType)ctx->wl_surface, NULL);
    if (ctx->surface == EGL_NO_SURFACE) {
        fprintf(stderr, "❌ EGL: Failed to create window surface\n");
        eglTerminate(ctx->display);
        free(ctx);
        return false;
    }

    const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    ctx->context = eglCreateContext(ctx->display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        fprintf(stderr, "❌ EGL: Failed to create OpenGL context\n");
        eglDestroySurface(ctx->display, ctx->surface);
        eglTerminate(ctx->display);
        free(ctx);
        return false;
    }

    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "❌ EGL: Failed to make context current\n");
        eglDestroyContext(ctx->display, ctx->context);
        eglDestroySurface(ctx->display, ctx->surface);
        eglTerminate(ctx->display);
        free(ctx);
        return false;
    }

    g_current_context = ctx;
    printf("✅ EGL + OpenGL context created on Wayland\n");
    return true;
}

static void egl_make_current(void) {
    if (g_current_context)
        eglMakeCurrent(g_current_context->display, g_current_context->surface,
                       g_current_context->surface, g_current_context->context);
}

static void egl_swap_buffers(void) {
    if (g_current_context)
        eglSwapBuffers(g_current_context->display, g_current_context->surface);
}

static void egl_destroy(void) {
    if (!g_current_context) return;
    eglMakeCurrent(g_current_context->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(g_current_context->display, g_current_context->context);
    eglDestroySurface(g_current_context->display, g_current_context->surface);
    eglTerminate(g_current_context->display);
    free(g_current_context);
    g_current_context = NULL;
    printf("✅ EGL OpenGL context destroyed\n");
}

GLContext* glcontext_create_for_platform(void* native_window, int width, int height) {
    if (!egl_init(native_window, width, height)) return NULL;
    return g_current_context;
}

void glcontext_destroy(void) { egl_destroy(); }
bool glcontext_is_initialized(void) { return g_current_context != NULL; }
void glcontext_set_current_context(GLContext* ctx) { g_current_context = ctx; }