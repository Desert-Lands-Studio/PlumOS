/* ── PlumX/PlumUI/src/render/rhi_registry.h ─────────────────────────────────────────── */
#pragma once
#include "rhi.h"

#ifdef __cplusplus
extern "C" {
#endif

PLUM_API void plum_rhi_register(const PlumRhiInfo* info);
PLUM_API PlumRhi* plum_rhi_create_highest_version(PlumRhiId id);
PLUM_API PlumRhi* plum_rhi_create_highest_version_by_name(const char* name);
PLUM_API PlumRhi* plum_rhi_create_from_chain(const char* chain);
PLUM_API PlumRhi* plum_rhi_autoselect_default(void);
PLUM_API PlumRhi* plum_rhi_create_specific_version(PlumRhiId id, int major, int minor);
PLUM_API void plum_rhi_init_registry(void);
PLUM_API void plum_rhi_shutdown_registry(void);

typedef PlumRhi* (*PlumRhiCreateForWindowFn)(const PlumRhiVersion* version, PdkWindow* window);

PlumRhi* plum_rhi_create_highest_version_for_window(PlumRhiId id, PdkWindow* window);
PlumRhi* plum_rhi_create_highest_version_by_name_for_window(const char* name, PdkWindow* window);
PlumRhi* plum_rhi_create_from_chain_for_window(const char* chain, PdkWindow* window);
PlumRhi* plum_rhi_autoselect_default_for_window(PdkWindow* window);

#ifdef __cplusplus
}
#endif
