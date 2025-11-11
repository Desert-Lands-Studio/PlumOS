/* ── PlumX/PlumUI/src/render/rhi.h ─────────────────────────────────────────── */
#ifndef PLUM_RHI_H
#define PLUM_RHI_H

#include "plumconfig.h"
#include <stdbool.h>

typedef enum {
    PLUM_RHI_NONE,
    PLUM_RHI_OPENGL,
    PLUM_RHI_VULKAN,
    PLUM_RHI_D3D12,
    PLUM_RHI_METAL,
    PLUM_RHI_OPENGLES,
    PLUM_RHI_OPENCL,
    PLUM_RHI_PIREON,
    PLUM_RHI_PLUMPAINT
} PlumRhiId;

typedef struct {
    int major;
    int minor;
    int patch;
} PlumRhiVersion;

typedef struct PlumRhi PlumRhi;

typedef PlumRhi* (*PlumRhiCreateFn)(const PlumRhiVersion*);
typedef void (*PlumRhiDestroyFn)(PlumRhi*);

typedef struct {
    PlumRhiId id;
    const char* name;
    PlumRhiVersion version;
    int priority;
    bool (*available)(void);
    PlumRhiCreateFn create;
    PlumRhiDestroyFn destroy;
} PlumRhiInfo;

typedef void (*PlumRhiDestroyFn)(PlumRhi*);

struct PlumRhi {
    const PlumRhiInfo* info;
    PlumRhiVersion version;
};

void plum_rhi_register(const PlumRhiInfo* info);
PlumRhi* plum_rhi_create_highest_version_by_name(const char* name);
PlumRhi* plum_rhi_create_from_chain(const char* chain);
PlumRhi* plum_rhi_select_best(void);
PlumRhi* plum_rhi_autoselect_default(void);
PlumRhi* plum_get_rhi_context(void);

#endif /* PLUM_RHI_H */
