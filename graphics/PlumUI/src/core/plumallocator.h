/* ── PlumX/PlumUI/src/core/plumllocator.h ─────────────────────────────────────────── */
#pragma once
#include <stddef.h>
void* plum_alloc(size_t sz);
void  plum_free(void* p);
void  plum_allocator_init(void);
void  plum_allocator_shutdown(void);
