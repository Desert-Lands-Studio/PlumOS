#ifndef PLUM_RHI_GL_BACKEND_H
#define PLUM_RHI_GL_BACKEND_H

#include "rhi.h"
#include "pdk/pdktypes.h"

PlumRhi* plum_rhi_create_highest_version_for_window(PlumRhiId id, PdkWindow* window);
PlumRhi* plum_rhi_create_highest_version_by_name_for_window(const char* name, PdkWindow* window);
PlumRhi* plum_rhi_create_from_chain_for_window(const char* chain, PdkWindow* window);
PlumRhi* plum_rhi_autoselect_default_for_window(PdkWindow* window);

#endif /* PLUM_RHI_GL_BACKEND_H */
