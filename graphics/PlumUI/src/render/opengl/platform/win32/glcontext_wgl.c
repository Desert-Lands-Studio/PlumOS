#define WIN32_LEAN_AND_MEAN
#include "render/opengl/glcontext.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/wglext.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);

struct GLContext {
    HDC hdc;
    HGLRC hglrc;
    HWND hwnd;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
};

static GLContext* g_current_context = NULL;


static bool load_wgl_extensions(HDC hdc) {
    
    HGLRC temp_ctx = wglCreateContext(hdc);
    if (!temp_ctx) return false;
    
    wglMakeCurrent(hdc, temp_ctx);
    
    
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp_ctx);
    
    return wglCreateContextAttribsARB && wglChoosePixelFormatARB;
}

static bool wgl_init(void* native_window, int width, int height) {
    HWND hwnd = (HWND)native_window;
    if (!hwnd || !IsWindow(hwnd)) {
        fprintf(stderr, "WGL: Invalid native window\n");
        return false;
    }
    
    GLContext* ctx = (GLContext*)calloc(1, sizeof(GLContext));
    if (!ctx) return false;
    
    ctx->hwnd = hwnd;
    ctx->hdc = GetDC(hwnd);
    if (!ctx->hdc) {
        fprintf(stderr, "WGL: Failed to get device context\n");
        free(ctx);
        return false;
    }
    
    
    if (!load_wgl_extensions(ctx->hdc)) {
        fprintf(stderr, "WGL: Failed to load extensions\n");
        ReleaseDC(hwnd, ctx->hdc);
        free(ctx);
        return false;
    }
    
    
    const int pixel_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        0
    };
    
    int pixel_format;
    UINT num_formats;
    
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = 
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    
    if (!wglChoosePixelFormatARB(ctx->hdc, pixel_attribs, NULL, 1, &pixel_format, &num_formats) || num_formats == 0) {
        fprintf(stderr, "WGL: Failed to choose pixel format with ARB\n");
        
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR), 1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0,
            PFD_MAIN_PLANE, 0, 0, 0, 0
        };
        
        pixel_format = ChoosePixelFormat(ctx->hdc, &pfd);
        if (!pixel_format) {
            fprintf(stderr, "WGL: Failed to choose fallback pixel format\n");
            ReleaseDC(hwnd, ctx->hdc);
            free(ctx);
            return false;
        }
    }
    
    if (!SetPixelFormat(ctx->hdc, pixel_format, NULL)) {
        fprintf(stderr, "WGL: Failed to set pixel format\n");
        ReleaseDC(hwnd, ctx->hdc);
        free(ctx);
        return false;
    }
    
    
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
    if (wglCreateContextAttribsARB) {
        int context_attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 6,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0
        };
        
        ctx->hglrc = wglCreateContextAttribsARB(ctx->hdc, 0, context_attribs);
    }
    
    
    if (!ctx->hglrc) {
        ctx->hglrc = wglCreateContext(ctx->hdc);
    }
    
    if (!ctx->hglrc) {
        fprintf(stderr, "WGL: Failed to create OpenGL context\n");
        ReleaseDC(hwnd, ctx->hdc);
        free(ctx);
        return false;
    }
    
    if (!wglMakeCurrent(ctx->hdc, ctx->hglrc)) {
        fprintf(stderr, "WGL: Failed to make context current\n");
        wglDeleteContext(ctx->hglrc);
        ReleaseDC(hwnd, ctx->hdc);
        free(ctx);
        return false;
    }
    
    
    ctx->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    ctx->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
        wglGetProcAddress("wglChoosePixelFormatARB");
    
    g_current_context = ctx;
    
    printf("✅ WGL Context initialized successfully\n");
    printf("✅ OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    printf("✅ OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("✅ OpenGL Version: %s\n", glGetString(GL_VERSION));
    
    return true;
}

static void wgl_make_current(void) {
    if (!g_current_context) return;
    wglMakeCurrent(g_current_context->hdc, g_current_context->hglrc);
}

static void wgl_swap_buffers(void) {
    if (!g_current_context) return;
    SwapBuffers(g_current_context->hdc);
}

static void wgl_destroy(void) {
    if (!g_current_context) return;

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_current_context->hglrc);
    ReleaseDC(g_current_context->hwnd, g_current_context->hdc);

    free(g_current_context);
    g_current_context = NULL;
    printf("✅ WGL Context destroyed\n");
}

static void* wgl_get_proc_address(const char* name) {
    return (void*)wglGetProcAddress(name);
}

GLContextInterface g_wgl_interface = {
    .init = wgl_init,
    .make_current = wgl_make_current,
    .swap_buffers = wgl_swap_buffers,
    .destroy = wgl_destroy,
    .get_proc_address = wgl_get_proc_address
};

GLContext* glcontext_create_for_platform(void* native_window, int width, int height) {
    if (!g_wgl_interface.init(native_window, width, height)) {
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
