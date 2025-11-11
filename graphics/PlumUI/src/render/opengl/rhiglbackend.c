#include "rhiglbackend.h"
#include "glcontext.h"
#include "pdk/pdkwindow.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    PlumRhi base;
    GLContext* gl_context;
    PdkWindow* window;  // ← сохраняем окно
} GLBackend;

static bool is_opengl_available(void);
static PlumRhi* create_opengl(const PlumRhiVersion* requested_version);
static PlumRhi* create_opengl_for_window(const PlumRhiVersion* requested_version, PdkWindow* window);
static void destroy_opengl(PlumRhi* rhi);

static const PlumRhiInfo g_gl_info = {
    PLUM_RHI_OPENGL,
    "OpenGL",
    {4, 6},
    80,
    is_opengl_available,
    create_opengl,
    destroy_opengl
};

static const PlumRhiInfo g_gl_info_for_window = {
    PLUM_RHI_OPENGL,
    "OpenGL",
    {4, 6},
    80,
    is_opengl_available,
    (PlumRhiCreateFn)create_opengl_for_window,  // ← кастим, чтобы совпал тип
    destroy_opengl
};

static bool is_opengl_available(void) {
    return true;
}

static PlumRhi* create_opengl(const PlumRhiVersion* requested_version) {
    fprintf(stderr, "❌ OpenGL: create_opengl() called without window — this is not supported.\n");
    return NULL;
}

static PlumRhi* create_opengl_for_window(const PlumRhiVersion* requested_version, PdkWindow* window) {
    if (!window) {
        fprintf(stderr, "❌ OpenGL: PdkWindow is NULL\n");
        return NULL;
    }

    if (requested_version->major > 4 || (requested_version->major == 4 && requested_version->minor > 6)) {
        fprintf(stderr, "❌ OpenGL: requested version %d.%d not supported\n",
                requested_version->major, requested_version->minor);
        return NULL;
    }

    GLBackend* backend = (GLBackend*)malloc(sizeof(GLBackend));
    if (!backend) return NULL;

    backend->window = window;
    backend->gl_context = glcontext_create_for_platform(window, 800, 600);  // ← передаём window!

    if (!backend->gl_context) {
        free(backend);
        return NULL;
    }

    backend->base.info = &g_gl_info_for_window;
    backend->base.version = *requested_version;

    printf("✅ Created OpenGL backend with version %d.%d\n", 
           requested_version->major, requested_version->minor);
    return &backend->base;
}

static void destroy_opengl(PlumRhi* rhi) {
    if (!rhi) return;
    GLBackend* backend = (GLBackend*)rhi;
    if (backend->gl_context) {
        glcontext_destroy();
    }
    free(backend);
}

__attribute__((constructor))
static void register_opengl(void) {
    plum_rhi_register(&g_gl_info_for_window);  // ← регистрируем версию с поддержкой окна
}