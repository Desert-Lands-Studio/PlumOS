#include <stdlib.h>
#include <plum/alloc.h>


void* malloc(size_t size) {
    return pl_malloc(size);
}

void free(void* ptr) {
    pl_free(ptr);
}

void* calloc(size_t num, size_t size) {
    return pl_calloc(num, size);
}

void* realloc(void* ptr, size_t size) {
    return pl_realloc(ptr, size);
}