#pragma once
#include <stdint.h>

#pragma pack(push, 1)

#define PLAM_MAGIC      0x504C414Du
#define PLAM_RES_MAGIC  0x504C4D52u
#define PLAM_FAT_MAGIC  0x504C4D46u

typedef struct { uint64_t off, sz; } plam_rva_t;

typedef enum : uint16_t {
    PLAM_CPU_NONE    = 0x0000,
    PLAM_CPU_X86_64  = 0x8664,
    PLAM_CPU_ARM64   = 0xAA64,
    PLAM_CPU_RISCV64 = 0x00F3,
    PLAM_CPU_UNKNOWN = 0xFFFF
} plam_cpu_t;

typedef enum : uint16_t {
    PLAM_CPU_X86_64_SSE4  = 0x0001,
    PLAM_CPU_ARM64_NEON   = 0x0100,
    PLAM_CPU_RISCV_VECTOR = 0x1000
} plam_cpu_subtype_t;

typedef enum : uint16_t {
    PLAM_FT_NONE     = 0x00,
    PLAM_FT_KERNEL   = 0x01,
    PLAM_FT_DRIVER   = 0x02,
    PLAM_FT_SHARED   = 0x03,
    PLAM_FT_APP      = 0x04,
    PLAM_FT_MODULE   = 0x05,
    PLAM_FT_BOOT     = 0x06,
    PLAM_FT_PLUGIN   = 0x07,
    PLAM_FT_OBJECT   = 0x08
} plam_file_type_t;

#define PLAM_SIG_ED25519  1
#define PLAM_SIG_ECDSA    2
#define PLAM_SIG_QUANTUM  3

typedef struct {
    uint8_t  sig_type;
    uint8_t  key_revocation;
    uint16_t cert_count;
    uint64_t timestamp;
    uint8_t  reserved[16];
} plam_sig_header_t;

typedef enum : uint16_t {
    PLAM_RES_ICON       = 0x0100,
    PLAM_RES_VERSION    = 0x0200,
    PLAM_RES_DEPENDENCY = 0x0300,
    PLAM_RES_STRING     = 0x0400,
    PLAM_RES_UI         = 0x0500,
    PLAM_RES_VENDOR     = 0xF000
} plam_res_type_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t  format;
    uint8_t  mip_levels;
    uint16_t reserved;
} plam_icon_info_t;

#define PLAM_SEC_READ   (1u << 0)
#define PLAM_SEC_WRITE  (1u << 1)
#define PLAM_SEC_EXEC   (1u << 2)
#define PLAM_SEC_NOBITS (1u << 3)
#define PLAM_SEC_RELOC  (1u << 4)

typedef struct {
    uint64_t name_off;
    uint32_t type;
    uint32_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint64_t entsize;
    uint32_t comp_alg;
    uint32_t reserved;
} plam_section_t;

#define PLAM_REL_NONE 0
#define PLAM_REL_64   1

typedef struct {
    uint64_t offset;
    uint32_t type;
    uint32_t sym_idx;
    int64_t  addend;
} plam_reloc_t;

typedef struct {
    uint64_t name_off;
    uint64_t value;
    uint64_t size;
    uint8_t  type;
    uint8_t  bind;
    uint16_t section_idx;
    uint32_t version;
} plam_symbol_t;

typedef struct {
    uint64_t begin_addr;
    uint64_t end_addr;
    uint64_t unwind_info_off;
    uint32_t flags;
} plam_unwind_entry_t;

#define PLAM_DEP_WEAK     (1u << 0)
#define PLAM_DEP_OPTIONAL (1u << 1)

typedef struct {
    uint64_t name_off;
    uint64_t version;
    uint8_t  uuid[16];
    uint32_t flags;
} plam_dependency_entry_t;

#define PLAM_COMP_NONE  0
#define PLAM_COMP_LZ4   1
#define PLAM_COMP_ZSTD  2
#define PLAM_COMP_LZMA  3
#define PLAM_COMP_BROTLI 4

#define PLAM_F_PIE            (1u << 0)
#define PLAM_F_ASLR           (1u << 1)
#define PLAM_F_NX_STACK       (1u << 2)
#define PLAM_F_NX_HEAP        (1u << 3)
#define PLAM_F_GUARD_CF       (1u << 4)
#define PLAM_F_SEH_SAFE       (1u << 5)
#define PLAM_F_ISOLATED_MEM   (1u << 6)
#define PLAM_F_DEBUG_STRIPPED (1u << 7)
#define PLAM_F_NO_REEXPORTS   (1u << 8)
#define PLAM_F_HW_ACCEL       (1u << 9)
#define PLAM_F_HOT_PATCHABLE  (1u << 10)

#define PLAM_RELRO_NONE 0
#define PLAM_RELRO_PART 1
#define PLAM_RELRO_FULL 2

typedef struct {
    plam_rva_t security;
    plam_rva_t loadcfg;
    plam_rva_t tls;
    plam_rva_t cfg;
    uint64_t   fat_off;
    uint32_t   fat_cnt;
    uint32_t   reserved;
} plam_directories_t;

typedef struct {
    uint16_t cpu_id;
    uint16_t abi_ver;
    uint32_t align_log2;
    uint64_t offset;
    uint64_t size;
} plam_fatarch_t;

typedef struct {
    uint32_t magic;
    uint16_t hdr_ver_major;
    uint16_t hdr_ver_minor;
    uint16_t file_type;
    uint16_t hdr_size;
    uint32_t flags;
    uint32_t hdr_crc32;

    uint64_t entry_off;
    uint16_t cpu_id;
    uint16_t cpu_sub;
    uint32_t abi_ver;
    uint64_t cpu_feat;

    plam_rva_t str_table;
    plam_rva_t sym_table;
    uint64_t   section_table_off;
    uint32_t   section_count;
    uint64_t   reloc_table_off;
    uint32_t   reloc_count;
    uint64_t   unwind_table_off;
    uint32_t   unwind_count;

    uint64_t import_off;
    uint64_t export_off;
    uint64_t symbol_table_off;
    uint32_t symbol_count;

    plam_rva_t resources;
    plam_rva_t debug;

    uint8_t  uuid[16];
    uint8_t  build_hash[32];
    uint64_t timestamp;

    uint32_t os_abi;
    uint32_t os_ver_min;
    uint32_t os_ver_sdk;
    uint16_t crypto_mode;
    uint16_t hash_type;
    uint16_t sig_scheme;
    uint8_t  relro_lvl;
    uint8_t  file_comp;

    plam_rva_t manifest;
    uint32_t   deps_cnt;
    uint32_t   res_cnt;

    uint32_t lang_mask;
    uint16_t tool_major;
    uint16_t tool_minor;
    uint16_t stdlib_ver;
    uint8_t  comp_model;
    uint8_t  lto_pgo_flags;

    plam_directories_t dirs;

    uint8_t reserved[16];
} plam_header_t;

typedef struct {
    plam_rva_t mods_dir;
    plam_rva_t l10n_table;
    plam_rva_t src_repo;
    uint32_t   abi_rev;
    uint32_t   build_flags;
} plam_manifest_ext_t;

typedef struct {
    uint32_t magic;
    uint16_t type;
    uint16_t flags;
    plam_rva_t blob;
    uint64_t orig_size;
    uint8_t  comp_alg;
    char     lang[6];
    uint8_t  hash[48];
    uint8_t  extra[6];
} plam_resource_t;

typedef struct {
    uint64_t mod_base, mod_size;
    uint64_t init_fn, fini_fn;
    uint32_t req_kernel_ver;
    uint32_t flags;
} plam_kernelmod_t;

#pragma pack(pop)