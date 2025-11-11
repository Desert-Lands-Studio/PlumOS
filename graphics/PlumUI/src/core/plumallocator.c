/* ── PlumX/PlumUI/src/core/plumllocator.c ─────────────────────────────────────────── */
#include "plumallocator.h"
#include <stdlib.h>
void  plum_allocator_init(void)     { }
void  plum_allocator_shutdown(void) { }
void* plum_alloc(size_t s)          { return malloc(s); }
void  plum_free(void* p)            { free(p); }
