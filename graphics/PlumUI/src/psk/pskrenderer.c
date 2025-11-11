/* ── PlumX/PlumUI/src/psk/pskrenderer.c ─────────────────────────────────────────── */
#include "psk.h"
#include "core/plumallocator.h"
#include <stdio.h>
#include <stdlib.h>

PskRenderer* psk_renderer_new(PlumRhi* rhi, PdkWindow* window) {
    PskRenderer* pr = (PskRenderer*)plum_alloc(sizeof(PskRenderer));
    if (!pr) return NULL;
    pr->rhi = rhi;
    pr->window = window;
    printf("Initializing renderer for window with RHI: %s\n", rhi ? rhi->info->name : "none");
    return pr;
}

void psk_renderer_free(PskRenderer* pr) {
    if (!pr) return;
    printf("Freeing renderer\n");
    plum_free(pr);
}

void psk_renderer_begin_frame(PskRenderer* pr) {
    if (!pr) return;
    printf("Beginning frame with RHI: %s\n", pr->rhi ? pr->rhi->info->name : "none");
}

void psk_renderer_end_frame(PskRenderer* pr) {
    if (!pr) return;
    printf("Ending frame with RHI: %s\n", pr->rhi ? pr->rhi->info->name : "none");
}
