/* ── PlumX/PlumUI/src/render/loader.h ─────────────────────────────────────────── */
#ifndef PLUM_LOADER_H
#define PLUM_LOADER_H

#include "plumconfig.h"
#include "rhi.h"

void plum_rhi_force(const char* name);
void plum_rhi_set_order(const char* order_string);
void plum_rhi_parse_args(int argc, char** argv);
void plum_rhi_parse_env(void);
PlumRhi* plum_rhi_select_by_order(void);

#endif /* PLUM_LOADER_H */
