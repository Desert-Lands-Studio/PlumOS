/* ── PlumX/PlumUI/src/core/plumbuffer.c ─────────────────────────────────────────── */
#include "plumbuffer.h"
#include "plumallocator.h"
void plum_buffer_init(PlumBuffer* b, size_t cap)
{ b->size = 0; b->capacity = cap; b->data = cap ? plum_alloc(cap) : NULL; }
void plum_buffer_free(PlumBuffer* b)
{ if(b->data) plum_free(b->data); b->data = NULL; b->size = b->capacity = 0; }
