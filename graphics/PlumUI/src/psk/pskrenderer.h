#ifndef PLUM_PSK_RENDERER_H
#define PLUM_PSK_RENDERER_H

#include "psk.h"


PskRenderer* psk_renderer_create(PlumRhi* rhi, PdkWindow* window);
void psk_renderer_destroy(PskRenderer* renderer);
void psk_renderer_begin_frame(PskRenderer* renderer);
void psk_renderer_end_frame(PskRenderer* renderer);

#endif
