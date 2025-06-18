#ifndef POM_FORMAT_H
#define POM_FORMAT_H
#include <stdint.h>

/* Магия «POM\0» */
#define POM_MAGIC 0x504F4D00u

/* Секция .pom.meta — TOML‑блок (offset,size) */
typedef struct {
    uint32_t magic;          /* POM_MAGIC */
    uint16_t arch;           /* plam_arch_t       */
    uint16_t abi_ver;
    uint32_t flags;          /*  bit0=LTO, bit1=PIC  */
    uint64_t meta_off, meta_size;   /* TOML с build‑info */
} pom_header_t;

#endif
