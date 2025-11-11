#include "plum.h"
#include "plumallocator.h"
#include "render/rhi_registry.h"
#include "pdk/pdkwindow.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    bool initialized;
    char forced_rhi[64];
    char selected_chain[128];
    PlumRhiId selected_id;
    char selected_name[32];
    bool use_chain;
    PlumRhiVersion requested_version;
} PlumGlobalState;

static PlumGlobalState g_plum = {0};
static PlumRhi* g_app_rhi = NULL;

void plum_set_default_rhi(const char* name) {
    if (!g_plum.initialized) {
        strncpy(g_plum.forced_rhi, name, sizeof(g_plum.forced_rhi) - 1);
        g_plum.forced_rhi[sizeof(g_plum.forced_rhi) - 1] = '\0';
    }
}

void plum_select_rhi_backend(PlumRhiId backend_id) {
    if (g_plum.initialized) {
        fprintf(stderr, "[Plum] RHI must be selected BEFORE initialization\n");
        return;
    }
    g_plum.selected_id = backend_id;
    g_plum.use_chain = false;
}

void plum_select_rhi_backend_by_name(const char* backend_name) {
    if (g_plum.initialized) {
        fprintf(stderr, "[Plum] RHI must be selected BEFORE initialization\n");
        return;
    }
    strncpy(g_plum.selected_name, backend_name, sizeof(g_plum.selected_name) - 1);
    g_plum.selected_name[sizeof(g_plum.selected_name) - 1] = '\0';
    g_plum.use_chain = false;
}

void plum_select_rhi_fallback_chain(const char* chain) {
    if (g_plum.initialized) {
        fprintf(stderr, "[Plum] RHI must be selected BEFORE initialization\n");
        return;
    }
    strncpy(g_plum.selected_chain, chain, sizeof(g_plum.selected_chain) - 1);
    g_plum.selected_chain[sizeof(g_plum.selected_chain) - 1] = '\0';
    g_plum.use_chain = true;
}

bool plum_init(void) {
    if (g_plum.initialized) return true;

    plum_allocator_init();
    plum_rhi_init_registry();
    g_plum.initialized = true;
    puts("[Plum] initialized (RHI deferred)");
    return true;
}

void plum_quit(void) {
    if (!g_plum.initialized) return;

    if (g_app_rhi) {
        if (g_app_rhi->info && g_app_rhi->info->destroy) {
            g_app_rhi->info->destroy(g_app_rhi);
        } else {
            plum_free(g_app_rhi);
        }
        g_app_rhi = NULL;
    }

    plum_rhi_shutdown_registry();
    plum_allocator_shutdown();
    puts("[Plum] shutdown");
    g_plum.initialized = false;
}

PlumRhi* plum_get_rhi_context(void) {
    return g_app_rhi;
}

void plum_internal_set_forced_rhi(const char* name) {
    if (!g_plum.initialized && name) {
        strncpy(g_plum.forced_rhi, name, sizeof(g_plum.forced_rhi) - 1);
        g_plum.forced_rhi[sizeof(g_plum.forced_rhi) - 1] = '\0';
    }
}

PLUM_API void plum_select_rhi_version(PlumRhiId backend_id, int major, int minor) {
    if (g_plum.initialized) {
        fprintf(stderr, "[Plum] RHI must be selected BEFORE initialization\n");
        return;
    }
    g_plum.selected_id = backend_id;
    g_plum.use_chain = false;
    g_plum.requested_version.major = major;
    g_plum.requested_version.minor = minor;
}

PLUM_API void plum_select_rhi_version_by_name(const char* backend_name, int major, int minor) {
    if (g_plum.initialized) {
        fprintf(stderr, "[Plum] RHI must be selected BEFORE initialization\n");
        return;
    }
    strncpy(g_plum.selected_name, backend_name, sizeof(g_plum.selected_name) - 1);
    g_plum.selected_name[sizeof(g_plum.selected_name) - 1] = '\0';
    g_plum.use_chain = false;
    g_plum.requested_version.major = major;
    g_plum.requested_version.minor = minor;
}

PlumRhi* plum_create_rhi_for_window(PdkWindow* window) {
    if (!window) return NULL;

    PlumRhi* rhi = NULL;

    if (!g_plum.use_chain && (g_plum.selected_id != PLUM_RHI_NONE || g_plum.selected_name[0] != '\0')) {
        if (g_plum.selected_id != PLUM_RHI_NONE) {
            rhi = plum_rhi_create_highest_version_for_window(g_plum.selected_id, window);
        } else {
            rhi = plum_rhi_create_highest_version_by_name_for_window(g_plum.selected_name, window);
        }
    }
    else if (g_plum.use_chain) {
        rhi = plum_rhi_create_from_chain_for_window(g_plum.selected_chain, window);
    }
    else if (g_plum.forced_rhi[0] != '\0') {
        rhi = plum_rhi_create_highest_version_by_name_for_window(g_plum.forced_rhi, window);
    }
    else {
        rhi = plum_rhi_autoselect_default_for_window(window);
    }

    if (rhi) {
        printf("[Plum] Using RHI: %s %d.%d\n",
               rhi->info->name,
               rhi->version.major,
               rhi->version.minor);
        g_app_rhi = rhi;  // Сохраняем в глобальный контекст
    }
    return rhi;
}

const char* plum_get_forced_rhi(void) {
    return g_plum.forced_rhi[0] ? g_plum.forced_rhi : NULL;
}