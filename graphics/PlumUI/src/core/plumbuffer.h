/* ── PlumX/PlumUI/src/core/plumbuffer.h ─────────────────────────────────────────── */
#pragma once
#include <stddef.h>
#include <stdint.h>
typedef struct {
    uint8_t* data;
    size_t   size, capacity;
} PlumBuffer;
void  plum_buffer_init(PlumBuffer*, size_t reserve);
void  plum_buffer_free(PlumBuffer*);
