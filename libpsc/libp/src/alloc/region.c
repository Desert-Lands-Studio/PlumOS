#include <plum/alloc.h>
#include <plum/syscalls.h>

struct pl_region {
    void*  start;
    size_t size;
    size_t used;
    pl_region_t* next;
};

pl_region_t* pl_region_create(size_t size) {
    void* memory = pl_syscall_mmap(NULL, size, PROT_READ | PROT_WRITE, 
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) return NULL;

    pl_region_t* region = (pl_region_t*)memory;
    region->start = memory;
    region->size = size;
    region->used = sizeof(pl_region_t);
    region->next = NULL;
    return region;
}

void* pl_region_alloc(pl_region_t* region, size_t size) {
    size = (size + 7) & ~7;
    if (region->used + size <= region->size) {
        void* ptr = (void*)((uintptr_t)region->start + region->used);
        region->used += size;
        return ptr;
    }
    return NULL;
}