/* ── PlumX/PlumUI/src/render/loader.c ─────────────────────────────────────────── */
#include "loader.h"
#include "plum.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void plum_rhi_force(const char* name) {
    plum_internal_set_forced_rhi(name);
}

void plum_rhi_set_order(const char* order_string) {
    (void)order_string;
}

void plum_rhi_parse_args(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-rhi") == 0 && i + 1 < argc) {
            plum_rhi_force(argv[i + 1]);
            i++;
        } else if (strncmp(argv[i], "-rhi:", 5) == 0) {
            plum_rhi_force(argv[i] + 5);
        } else if (strcmp(argv[i], "-force_vulkan") == 0) {
            plum_rhi_force("Vulkan");
        } else if (strcmp(argv[i], "-force_opengl") == 0) {
            plum_rhi_force("OpenGL");
        } else if (strcmp(argv[i], "-force_opengles") == 0) {
            plum_rhi_force("OpenGLES");
        } else if (strcmp(argv[i], "-force_d3d12") == 0) {
            plum_rhi_force("D3D12");
        } else if (strcmp(argv[i], "-force_metal") == 0) {
            plum_rhi_force("Metal");
        } else if (strcmp(argv[i], "-rhi_chain") == 0 && i + 1 < argc) {
            (void)argv[i + 1];
            i++;
        }
    }
}

void plum_rhi_parse_env(void) {
    const char* force = getenv("PLUM_FORCE_RHI");
    if (force) {
        plum_rhi_force(force);
    }
}