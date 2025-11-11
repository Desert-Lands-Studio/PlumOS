#include "render/opengl/glcontext.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct GLContext {
    Display* display;
    Window window;
    GLXContext context;
    XVisualInfo* vi;
    GLXFBConfig fbconfig;
};

static GLContext* g_current_context = NULL;

static GLXFBConfig get_fbconfig_from_visual(Display* display, XVisualInfo* vi) {
    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), NULL, &fbcount);
    if (!fbc) return NULL;

    GLXFBConfig result = NULL;
    for (int i = 0; i < fbcount; i++) {
        XVisualInfo* visual = glXGetVisualFromFBConfig(display, fbc[i]);
        if (visual && visual->visualid == vi->visualid) {
            result = fbc[i];
            XFree(visual);
            break;
        }
        if (visual) XFree(visual);
    }
    
    XFree(fbc);
    return result;
}

static bool glx_init(void* native_window, int width, int height) {
    Window x11_window = (Window)(uintptr_t)native_window;
    if (!x11_window) {
        fprintf(stderr, "GLX: Invalid native window\n");
        return false;
    }

    GLContext* ctx = (GLContext*)calloc(1, sizeof(GLContext));
    if (!ctx) return false;

    ctx->display = XOpenDisplay(NULL);
    if (!ctx->display) {
        fprintf(stderr, "GLX: Failed to open display\n");
        free(ctx);
        return false;
    }

    int attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    int fb_attribs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        None
    };

    int glx_major, glx_minor;
    if (!glXQueryVersion(ctx->display, &glx_major, &glx_minor) ||
        (glx_major == 1 && glx_minor < 3)) {
        fprintf(stderr, "GLX: Version 1.3 or higher required\n");
        XCloseDisplay(ctx->display);
        free(ctx);
        return false;
    }

    ctx->vi = glXChooseVisual(ctx->display, DefaultScreen(ctx->display), fb_attribs);
    if (!ctx->vi) {
        fprintf(stderr, "GLX: Failed to choose visual\n");
        XCloseDisplay(ctx->display);
        free(ctx);
        return false;
    }

    ctx->fbconfig = get_fbconfig_from_visual(ctx->display, ctx->vi);

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
    glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
        glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

    if (glXCreateContextAttribsARB && ctx->fbconfig) {
        ctx->context = glXCreateContextAttribsARB(ctx->display, ctx->fbconfig, NULL, True, attribs);
    } else {
        ctx->context = glXCreateContext(ctx->display, ctx->vi, NULL, GL_TRUE);
    }

    if (!ctx->context) {
        fprintf(stderr, "GLX: Failed to create context\n");
        XFree(ctx->vi);
        XCloseDisplay(ctx->display);
        free(ctx);
        return false;
    }

    if (!glXMakeCurrent(ctx->display, x11_window, ctx->context)) {
        fprintf(stderr, "GLX: Failed to make context current\n");
        glXDestroyContext(ctx->display, ctx->context);
        XFree(ctx->vi);
        XCloseDisplay(ctx->display);
        free(ctx);
        return false;
    }

    ctx->window = x11_window;
    g_current_context = ctx;
    printf("✅ GLX Context initialized successfully\n");
    return true;
}

static void glx_make_current(void) {
    if (!g_current_context) return;
    glXMakeCurrent(g_current_context->display, g_current_context->window, g_current_context->context);
}

static void glx_swap_buffers(void) {
    if (!g_current_context) return;
    glXSwapBuffers(g_current_context->display, g_current_context->window);
}

static void glx_destroy(void) {
    if (!g_current_context) return;

    glXMakeCurrent(g_current_context->display, None, NULL);
    glXDestroyContext(g_current_context->display, g_current_context->context);
    XFree(g_current_context->vi);
    XCloseDisplay(g_current_context->display);

    free(g_current_context);
    g_current_context = NULL;
    printf("✅ GLX Context destroyed\n");
}

static void* glx_get_proc_address(const char* name) {
    return (void*)glXGetProcAddress((const GLubyte*)name);
}

GLContextInterface g_glx_interface = {
    .init = glx_init,
    .make_current = glx_make_current,
    .swap_buffers = glx_swap_buffers,
    .destroy = glx_destroy,
    .get_proc_address = glx_get_proc_address
};

GLContext* glcontext_create_for_platform(void* native_window, int width, int height) {
    if (!g_glx_interface.init(native_window, width, height)) {
        return NULL;
    }
    return g_current_context;
}

void glcontext_destroy(void) {
    if (g_current_context) {
        glx_destroy();
    }
}

bool glcontext_is_initialized(void) {
    return g_current_context != NULL;
}

void glcontext_set_current_context(GLContext* ctx) {
    g_current_context = ctx;
}
