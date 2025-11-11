
#include "render/opengl/glcontext.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <wayland-client.h>
#include <stdlib.h>
#include <stdio.h>
#include "pdk/pdkwindow.h"

struct GLContext {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    struct wl_surface* wl_surface;
};

static struct GLContext* g_current_context = NULL;

static bool gles_init(void* native_window, int width, int height) {
    PdkWindow* pdk_win = (PdkWindow*)native_window;
    if (!pdk_win || !pdk_win->surface) {
        fprintf(stderr, "❌ GLES: Invalid PdkWindow or surface\n");
        return false;
    }

    struct GLContext* ctx = calloc(1, sizeof(struct GLContext));
    if (!ctx) return false;

    ctx->wl_surface = pdk_win->surface;
    ctx->display = eglGetDisplay((EGLNativeDisplayType)pdk_win->display);
    if (ctx->display == EGL_NO_DISPLAY) goto fail;

    if (!eglInitialize(ctx->display, NULL, NULL)) {
        fprintf(stderr, "❌ GLES: eglInitialize failed\n");
        goto fail;
    }

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num;
    if (!eglChooseConfig(ctx->display, attribs, &config, 1, &num) || num == 0) {
        fprintf(stderr, "❌ GLES: Failed to choose config\n");
        goto fail;
    }

    ctx->surface = eglCreateWindowSurface(ctx->display, config, (EGLNativeWindowType)ctx->wl_surface, NULL);
    if (ctx->surface == EGL_NO_SURFACE) {
        fprintf(stderr, "❌ GLES: Failed to create window surface\n");
        goto fail;
    }

    const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    ctx->context = eglCreateContext(ctx->display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        fprintf(stderr, "❌ GLES: Failed to create context\n");
        goto fail;
    }

    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "❌ GLES: Failed to make context current\n");
        goto fail;
    }

    g_current_context = ctx;
    printf("✅ OpenGL ES 2.0 context created on Wayland\n");
    return true;

fail:
    if (ctx->context) eglDestroyContext(ctx->display, ctx->context);
    if (ctx->surface) eglDestroySurface(ctx->display, ctx->surface);
    if (ctx->display != EGL_NO_DISPLAY) eglTerminate(ctx->display);
    free(ctx);
    return false;
}

static void gles_make_current(void) {
    if (g_current_context) {
        eglMakeCurrent(g_current_context->display,
                       g_current_context->surface,
                       g_current_context->surface,
                       g_current_context->context);
    }
}

static void gles_swap_buffers(void) {
    if (g_current_context) {
        eglSwapBuffers(g_current_context->display, g_current_context->surface);
    }
}

static void gles_destroy(void) {
    if (!g_current_context) return;

    eglMakeCurrent(g_current_context->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(g_current_context->display, g_current_context->context);
    eglDestroySurface(g_current_context->display, g_current_context->surface);
    eglTerminate(g_current_context->display);

    free(g_current_context);
    g_current_context = NULL;
    printf("✅ OpenGL ES context destroyed\n");
}

static void* gles_get_proc_address(const char* name) {
    return (void*)eglGetProcAddress(name);
}


GLContextInterface g_gles_interface = {
    .init = gles_init,
    .make_current = gles_make_current,
    .swap_buffers = gles_swap_buffers,
    .destroy = gles_destroy,
    .get_proc_address = gles_get_proc_address
};


GLContext* glcontext_create_for_platform(void* native_window, int width, int height) {
    if (!g_gles_interface.init(native_window, width, height)) {
        return NULL;
    }
    return g_current_context;
}

void glcontext_destroy(void) {
    if (g_current_context) {
        g_gles_interface.destroy();
    }
}

bool glcontext_is_initialized(void) {
    return g_current_context != NULL;
}

void glcontext_set_current_context(GLContext* ctx) {
    g_current_context = ctx;
}