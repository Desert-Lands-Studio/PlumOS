/* ── PlumX/PlumUI/src/psk/psk.h ─────────────────────────────────────────── */
#ifndef PLUM_PSK_H
#define PLUM_PSK_H

#include "render/rhi.h"
#include "pdk/pdktypes.h"

typedef struct PskRenderer {
    PlumRhi* rhi;
    PdkWindow* window;
} PskRenderer;

PskRenderer* psk_renderer_new(PlumRhi* rhi, PdkWindow* window);
void psk_renderer_free(PskRenderer* pr);
void psk_renderer_begin_frame(PskRenderer* pr);
void psk_renderer_end_frame(PskRenderer* pr);

#endif /* PLUM_PSK_H */
