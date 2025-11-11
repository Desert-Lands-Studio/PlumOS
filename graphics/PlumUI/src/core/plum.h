/* ── PlumX/PlumUI/src/core/plum.h ─────────────────────────────────────────── */
#pragma once
#include <stdbool.h>
#include "plumconfig.h"
#include "render/rhi.h"

#ifdef __cplusplus
extern "C" {
#endif

PLUM_API bool plum_init(void);
PLUM_API void plum_quit(void);
PLUM_API void plum_set_default_rhi(const char* name);
PLUM_API void plum_select_rhi_backend(PlumRhiId backend_id);
PLUM_API void plum_select_rhi_backend_by_name(const char* backend_name);
PLUM_API void plum_select_rhi_fallback_chain(const char* chain);
PLUM_API PlumRhi* plum_get_rhi_context(void);
PLUM_API const char* plum_get_forced_rhi(void);
PLUM_API void plum_internal_set_forced_rhi(const char* name);

#ifdef __cplusplus
}
#endif
