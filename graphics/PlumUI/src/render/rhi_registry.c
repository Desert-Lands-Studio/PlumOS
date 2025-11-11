/* ── PlumX/PlumUI/src/render/rhi_registry.c ─────────────────────────────────────────── */
#include "rhi_registry.h"
#include "plumallocator.h"
#include "core/plum.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BACKENDS 32

static const PlumRhiInfo* g_backends[MAX_BACKENDS];
static size_t g_backend_count = 0;
static bool g_initialized = false;

void plum_rhi_register(const PlumRhiInfo* info) {
    if (g_backend_count >= MAX_BACKENDS || !info || !info->available) return;
    g_backends[g_backend_count++] = info;
}

PlumRhi* plum_rhi_create_highest_version(PlumRhiId id) {
    const PlumRhiInfo* best_info = NULL;
    PlumRhiVersion max_version = {0, 0};
    
    
    for (size_t i = 0; i < g_backend_count; i++) {
        if (g_backends[i]->id == id) {
            if (g_backends[i]->version.major > max_version.major || 
               (g_backends[i]->version.major == max_version.major && 
                g_backends[i]->version.minor > max_version.minor)) {
                best_info = g_backends[i];
                max_version = g_backends[i]->version;
            }
        }
    }
    
    if (!best_info) return NULL;
    
    
    PlumRhi* rhi = best_info->create(&max_version);
    if (rhi) {
        rhi->version = max_version;
        return rhi;
    }
    
    
    PlumRhiVersion try_version = max_version;
    while (try_version.major > 0 || try_version.minor > 0) {
        
        if (try_version.minor > 0) {
            try_version.minor--;
        } else {
            try_version.major--;
            try_version.minor = 99; 
        }
        
        
        if (try_version.major > max_version.major || 
           (try_version.major == max_version.major && 
            try_version.minor > max_version.minor)) {
            continue;
        }
        
        
        for (size_t i = 0; i < g_backend_count; i++) {
            if (g_backends[i]->id == id && 
                g_backends[i]->version.major == try_version.major &&
                g_backends[i]->version.minor >= try_version.minor) {
                rhi = g_backends[i]->create(&try_version);
                if (rhi) {
                    rhi->version = try_version;
                    return rhi;
                }
            }
        }
    }
    
    return NULL;
}

PlumRhi* plum_rhi_create_highest_version_by_name(const char* name) {
    PlumRhiId id = PLUM_RHI_NONE;
    
    if (strcmp(name, "Vulkan") == 0) id = PLUM_RHI_VULKAN;
    else if (strcmp(name, "OpenGL") == 0) id = PLUM_RHI_OPENGL;
    else if (strcmp(name, "OpenGLES") == 0) id = PLUM_RHI_OPENGLES;
    else if (strcmp(name, "D3D12") == 0) id = PLUM_RHI_D3D12;
    else if (strcmp(name, "Metal") == 0) id = PLUM_RHI_METAL;
    
    if (id == PLUM_RHI_NONE) {
        fprintf(stderr, "Unknown RHI backend: %s\n", name);
        return NULL;
    }
    
    return plum_rhi_create_highest_version(id);
}

PlumRhi* plum_rhi_create_from_chain(const char* chain) {
    if (!chain || !*chain) return NULL;
    
    char* chain_copy = strdup(chain);
    if (!chain_copy) return NULL;
    
    char* token = strtok(chain_copy, ",>");
    PlumRhi* rhi = NULL;
    
    while (token) {
        
        while (*token == ' ') token++;
        
        rhi = plum_rhi_create_highest_version_by_name(token);
        if (rhi) break;
        
        token = strtok(NULL, ",>");
    }
    
    free(chain_copy);
    return rhi;
}

PlumRhi* plum_rhi_create_highest_version_by_name_for_window(const char* name, PdkWindow* window) {
    PlumRhiId id = PLUM_RHI_NONE;
    if (strcmp(name, "Vulkan") == 0) id = PLUM_RHI_VULKAN;
    else if (strcmp(name, "OpenGL") == 0) id = PLUM_RHI_OPENGL;
    else if (strcmp(name, "OpenGLES") == 0) id = PLUM_RHI_OPENGLES;
    else if (strcmp(name, "D3D12") == 0) id = PLUM_RHI_D3D12;
    else if (strcmp(name, "Metal") == 0) id = PLUM_RHI_METAL;
    if (id == PLUM_RHI_NONE) {
        fprintf(stderr, "Unknown RHI backend: %s\n", name);
        return NULL;
    }
    return plum_rhi_create_highest_version_for_window(id, window);
}

PlumRhi* plum_rhi_autoselect_default(void) {
    
    const char* default_chain = "D3D12,Vulkan,OpenGL,Metal";
    PlumRhi* rhi = plum_rhi_create_from_chain(default_chain);
    
    if (!rhi) {
        
        rhi = plum_rhi_create_highest_version(PLUM_RHI_OPENGLES);
    }
    
    return rhi;
}

PlumRhi* plum_rhi_create_specific_version(PlumRhiId id, int major, int minor) {
    PlumRhiVersion requested = {major, minor};
    
    for (size_t i = 0; i < g_backend_count; i++) {
        if (g_backends[i]->id == id) {
            PlumRhi* rhi = g_backends[i]->create(&requested);
            if (rhi) {
                printf("Created %s version %d.%d\n", 
                       g_backends[i]->name, major, minor);
                return rhi;
            }
        }
    }
    
    
    return plum_rhi_create_highest_version(id);
}

void plum_rhi_init_registry(void) {
    if (g_initialized) return;
    g_initialized = true;
    
    
    
}

void plum_rhi_shutdown_registry(void) {
    g_backend_count = 0;
    g_initialized = false;
}

PlumRhi* plum_rhi_create_highest_version_for_window(PlumRhiId id, PdkWindow* window) {
    if (!window) return NULL;
    const PlumRhiInfo* best_info = NULL;
    PlumRhiVersion max_version = {0, 0};
    for (size_t i = 0; i < g_backend_count; i++) {
        if (g_backends[i]->id == id) {
            if (g_backends[i]->version.major > max_version.major ||
                (g_backends[i]->version.major == max_version.major &&
                 g_backends[i]->version.minor > max_version.minor)) {
                best_info = g_backends[i];
                max_version = g_backends[i]->version;
            }
        }
    }
    if (!best_info || !best_info->create) return NULL;

    PlumRhiCreateForWindowFn create_for_window = (PlumRhiCreateForWindowFn)best_info->create;
    PlumRhi* rhi = create_for_window(&max_version, window);
    if (rhi) {
        rhi->version = max_version;
        return rhi;
    }
    return NULL;
}

PlumRhi* plum_rhi_create_highest_version_by_name_for_window(const char* name, PdkWindow* window) {
    PlumRhiId id = PLUM_RHI_NONE;
    if (strcmp(name, "Vulkan") == 0) id = PLUM_RHI_VULKAN;
    else if (strcmp(name, "OpenGL") == 0) id = PLUM_RHI_OPENGL;
    else if (strcmp(name, "OpenGLES") == 0) id = PLUM_RHI_OPENGLES;
    else if (strcmp(name, "D3D12") == 0) id = PLUM_RHI_D3D12;
    else if (strcmp(name, "Metal") == 0) id = PLUM_RHI_METAL;
    if (id == PLUM_RHI_NONE) {
        fprintf(stderr, "Unknown RHI backend: %s\n", name);
        return NULL;
    }
    return plum_rhi_create_highest_version_for_window(id, window);
}

PlumRhi* plum_rhi_create_from_chain_for_window(const char* chain, PdkWindow* window) {
    if (!chain || !*chain || !window) return NULL;
    char* chain_copy = strdup(chain);
    if (!chain_copy) return NULL;
    char* token = strtok(chain_copy, ",>");
    PlumRhi* rhi = NULL;
    while (token) {
        while (*token == ' ') token++;
        rhi = plum_rhi_create_highest_version_by_name_for_window(token, window);
        if (rhi) break;
        token = strtok(NULL, ",>");
    }
    free(chain_copy);
    return rhi;
}

PlumRhi* plum_rhi_autoselect_default_for_window(PdkWindow* window) {
    const char* default_chain = PLUM_CONF_RHI_DEFAULT;
    PlumRhi* rhi = plum_rhi_create_from_chain_for_window(default_chain, window);
    if (!rhi) {
        rhi = plum_rhi_create_highest_version_for_window(PLUM_RHI_OPENGLES, window);
    }
    return rhi;
}