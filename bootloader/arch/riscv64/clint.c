#include <stdint.h>


#define CLINT_BASE 0x2000000UL
#define CLINT_MTIMECMP (CLINT_BASE + 0x4000)
#define CLINT_MTIME (CLINT_BASE + 0xbff8)

static inline void clint_write(uint64_t reg, uint64_t data) {
    *(volatile uint64_t *)(reg) = data;
}

static inline uint64_t clint_read(uint64_t reg) {
    return *(volatile uint64_t *)(reg);
}

void clint_init() {
    
    clint_write(CLINT_MTIMECMP, clint_read(CLINT_MTIME) + 1000000); 
}

uint64_t clint_get_time() {
    return clint_read(CLINT_MTIME);
}