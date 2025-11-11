#pragma once
#include <stddef.h>
#include <stdint.h>


void* pl_malloc(size_t size);
void pl_free(void* ptr);
void* pl_calloc(size_t num, size_t size);
void* pl_realloc(void* ptr, size_t size);


typedef struct pl_region pl_region_t;
pl_region_t* pl_region_create(size_t size);
void pl_region_destroy(pl_region_t* region);
void* pl_region_alloc(pl_region_t* region, size_t size);
void pl_region_reset(pl_region_t* region);