#pragma once
#include <stdint.h>

/*====================================================================*
 *  PLAM_FORMAT — Portable Linking And Modules (v3.0.3)
 *  --------------------------------------------------
 *  On-disk format for PlumOS: kernels, drivers, apps, resources.
 *  All structures are 1-byte packed for cross-platform compatibility.
 *  CPU feature flags are architecture-specific to avoid collisions.
 *====================================================================*/

#pragma pack(push, 1)   /* 1-byte packing */

/*-------------------------------- Magic numbers -------------------------*/
#define PLAM_MAGIC      0x504C414Du  /* "PLAM" */
#define PLAM_FAT_MAGIC  0x504C4D46u  /* "PLMF" */
#define PLAM_RES_MAGIC  0x504C4D52u  /* "PLMR" */

/*-------------------------------- Versioning ---------------------------*/
#define PLAM_VERSION_MAJOR 3
#define PLAM_VERSION_MINOR 0
#define PLAM_VERSION_PATCH 3

/*-------------------------------- Generic helpers -----------------------*/
typedef struct { uint64_t off, sz; } plam_rva_t;   /* Offset + size */

/*-------------------------------- CPU architecture ----------------------*/
typedef enum : uint16_t {
    PLAM_CPU_NONE    = 0x0000,
    PLAM_CPU_X86_64  = 0x8664,      
    PLAM_CPU_ARM64   = 0xAA64,      
    PLAM_CPU_RISCV64 = 0x00F3,      
    PLAM_CPU_PRUM64  = 0x7072,
    PLAM_CPU_UNKNOWN = 0xFFFF
} plam_cpu_t;

/* CPU sub-features (bit-mask per arch; no cross-arch collisions) */
typedef enum : uint64_t {
    
    PLAM_CPU_X86_64_SSE      = 1ULL <<  0,
    PLAM_CPU_X86_64_SSE2     = 1ULL <<  1,
    PLAM_CPU_X86_64_SSE3     = 1ULL <<  2,
    PLAM_CPU_X86_64_SSSE3    = 1ULL <<  3,
    PLAM_CPU_X86_64_SSE4_1   = 1ULL <<  4,
    PLAM_CPU_X86_64_SSE4_2   = 1ULL <<  5,
    PLAM_CPU_X86_64_AVX      = 1ULL <<  6,
    PLAM_CPU_X86_64_F16C     = 1ULL <<  7,
    PLAM_CPU_X86_64_FMA      = 1ULL <<  8,
    PLAM_CPU_X86_64_AVX2     = 1ULL <<  9,
    PLAM_CPU_X86_64_BMI1     = 1ULL << 10,
    PLAM_CPU_X86_64_BMI2     = 1ULL << 11,
    PLAM_CPU_X86_64_ADX      = 1ULL << 12,
    PLAM_CPU_X86_64_RDSEED   = 1ULL << 13,
    PLAM_CPU_X86_64_SHA      = 1ULL << 14,
    PLAM_CPU_X86_64_AVX512F  = 1ULL << 15,
    PLAM_CPU_X86_64_AVX512DQ = 1ULL << 16,
    PLAM_CPU_X86_64_AVX512IFMA = 1ULL << 17,
    PLAM_CPU_X86_64_AVX512PF = 1ULL << 18,
    PLAM_CPU_X86_64_AVX512ER = 1ULL << 19,
    PLAM_CPU_X86_64_AVX512CD = 1ULL << 20,
    PLAM_CPU_X86_64_AVX512BW = 1ULL << 21,
    PLAM_CPU_X86_64_AVX512VL = 1ULL << 22,
    PLAM_CPU_X86_64_AVX512VBMI = 1ULL << 23,
    PLAM_CPU_X86_64_AVX512VNNI = 1ULL << 24,
    PLAM_CPU_X86_64_VAES     = 1ULL << 25,
    PLAM_CPU_X86_64_VPCLMULQDQ = 1ULL << 26,
    PLAM_CPU_X86_64_GFNI     = 1ULL << 27,
    PLAM_CPU_X86_64_SHSTK    = 1ULL << 28,
    PLAM_CPU_X86_64_PCONFIG  = 1ULL << 29,
    PLAM_CPU_X86_64_LAM      = 1ULL << 30,
    PLAM_CPU_X86_64_LBR      = 1ULL << 31,

    
    PLAM_CPU_ARM64_NEON      = 1ULL << 32,
    PLAM_CPU_ARM64_SVE       = 1ULL << 33,
    PLAM_CPU_ARM64_SVE2      = 1ULL << 34,
    PLAM_CPU_ARM64_SVE_BF16  = 1ULL << 35,
    PLAM_CPU_ARM64_SVE_I8MM  = 1ULL << 36,
    PLAM_CPU_ARM64_LSE       = 1ULL << 37,
    PLAM_CPU_ARM64_CRC32     = 1ULL << 38,
    PLAM_CPU_ARM64_SHA1_SHA2 = 1ULL << 39,
    PLAM_CPU_ARM64_SHA3      = 1ULL << 40,
    PLAM_CPU_ARM64_SM4       = 1ULL << 41,
    PLAM_CPU_ARM64_DIT       = 1ULL << 42,
    PLAM_CPU_ARM64_PAUTH     = 1ULL << 43,
    PLAM_CPU_ARM64_MTE       = 1ULL << 44,
    PLAM_CPU_ARM64_SME       = 1ULL << 45,
    PLAM_CPU_ARM64_VHE       = 1ULL << 46,
    PLAM_CPU_ARM64_SB        = 1ULL << 47,

    
    PLAM_CPU_RISCV_VECTOR    = 1ULL << 48,
    PLAM_CPU_RISCV_ZFH       = 1ULL << 49,
    PLAM_CPU_RISCV_ZFBFMIN   = 1ULL << 50,
    PLAM_CPU_RISCV_ZB        = 1ULL << 51,
    PLAM_CPU_RISCV_ZBB       = 1ULL << 52,
    PLAM_CPU_RISCV_ZK        = 1ULL << 53,
    PLAM_CPU_RISCV_ZVKB      = 1ULL << 54,
    PLAM_CPU_RISCV_ZVBC      = 1ULL << 55,
    PLAM_CPU_RISCV_ZAAMO     = 1ULL << 56,
    PLAM_CPU_RISCV_SVADU     = 1ULL << 57,
    PLAM_CPU_RISCV_SV57      = 1ULL << 58,
    PLAM_CPU_RISCV_H         = 1ULL << 59,
    PLAM_CPU_RISCV_SQOSID    = 1ULL << 60,
    PLAM_CPU_RISCV_SVINVAL   = 1ULL << 61,
    PLAM_CPU_RISCV_ZVAMACC   = 1ULL << 62,
    PLAM_CPU_RISCV_ZVMMUL    = 1ULL << 63,

} plam_cpu_subtype_t;

/*-------------------------------- File types ----------------------------*/
typedef enum : uint16_t {
    PLAM_FT_NONE     = 0x00,
    PLAM_FT_KERNEL   = 0x01,
    PLAM_FT_DRIVER   = 0x02,
    PLAM_FT_SHARED   = 0x03,
    PLAM_FT_APP      = 0x04,
    PLAM_FT_MODULE   = 0x05,
    PLAM_FT_BOOT     = 0x06,
    PLAM_FT_PLUGIN   = 0x07,
    PLAM_FT_OBJECT   = 0x08,
    PLAM_FT_FIRMWARE = 0x09,
    PLAM_FT_RESOURCE_ONLY = 0x0A
} plam_file_type_t;

/*-------------------------------- Program Header Types ------------------*/
#define PLAM_PT_NULL         0
#define PLAM_PT_LOAD         1
#define PLAM_PT_DYNAMIC      2
#define PLAM_PT_INTERP       3
#define PLAM_PT_NOTE         4
#define PLAM_PT_SHLIB        5
#define PLAM_PT_PHDR         6
#define PLAM_PT_TLS          7
#define PLAM_PT_LOOS         0x60000000
#define PLAM_PT_HIOS         0x6FFFFFFF
#define PLAM_PT_LOPROC       0x70000000
#define PLAM_PT_HIPROC       0x7FFFFFFF

/* Program Header Flags */
#define PLAM_PF_X            (1 << 0)  /* Execute */
#define PLAM_PF_W            (1 << 1)  /* Write */
#define PLAM_PF_R            (1 << 2)  /* Read */

/*-------------------------------- Security ------------------------------*/
#define PLAM_SIG_ED25519  1
#define PLAM_SIG_ECDSA    2
#define PLAM_SIG_FALCON   5
#define PLAM_SIG_SPHINCS  4

#define PLAM_HASH_SHA256   1
#define PLAM_HASH_SHA384   2
#define PLAM_HASH_SHA512   3
#define PLAM_HASH_BLAKE3   4
#define PLAM_HASH_SHA3_512 5

typedef struct {
    uint8_t  sig_type;
    uint8_t  hash_alg;
    uint8_t  sig_len;
    uint16_t key_rev;
    uint8_t  key_revocation;
    uint16_t cert_count;
    uint64_t timestamp;
    uint8_t  sig_type_v2;
    uint8_t  cert_chain_off;
    uint16_t attestation_flags;
    uint64_t tpm_quote;
    uint64_t sig_data_off;
    uint64_t custom_oid_off;  /* NEW: Offset to custom OID string (for extensible algos) */
    uint8_t  reserved[6 - sizeof(uint64_t)];  /* Adjusted reserved */
} plam_sig_header_t;

/*-------------------------------- Resources -----------------------------*/
typedef enum : uint16_t {
    PLAM_RES_ICON        = 0x0100,
    PLAM_RES_VERSION     = 0x0200,
    PLAM_RES_DEPENDENCY  = 0x0300,
    PLAM_RES_STRING      = 0x0400,
    PLAM_RES_UI          = 0x0500,
    PLAM_RES_PERMISSIONS = 0x0600,
    PLAM_RES_MANIFEST    = 0x0700,
    PLAM_RES_LOCALIZATION= 0x0800,
    PLAM_RES_CONFIG      = 0x0900,
    PLAM_RES_UI_LAYOUT   = 0x0A00,
    PLAM_RES_DEVICE_TREE = 0x0B00,
    PLAM_RES_VM_CONFIG   = 0x0C00,
    PLAM_RES_SUBSYS_MANIFEST = 0x0D00,
    PLAM_RES_VENDOR      = 0xF000
} plam_res_type_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t  format;
    uint8_t  mip_levels;
    uint16_t flags;
} plam_icon_info_t;

typedef struct {
    uint64_t cap_flags[4];
} plam_permissions_t;

/*-------------------------------- Section types -------------------------*/
#define PLAM_SHT_NULL        0
#define PLAM_SHT_PROGBITS    1
#define PLAM_SHT_SYMTAB      2
#define PLAM_SHT_STRTAB      3
#define PLAM_SHT_RELA        4
#define PLAM_SHT_HASH        5
#define PLAM_SHT_DYNAMIC     6
#define PLAM_SHT_NOTE        7

#define PLAM_SHT_ACCEL_CODE  0x80000001
#define PLAM_SHT_ACCEL_DATA  0x80000002
#define PLAM_SHT_ACCEL_CFG   0x80000003
#define PLAM_SHT_WASM_CODE   0x80000010
#define PLAM_SHT_METADATA    0x800000FF

/*-------------------------------- Section table -------------------------*/
#define PLAM_SEC_READ     (1u << 0)
#define PLAM_SEC_WRITE    (1u << 1)
#define PLAM_SEC_EXEC     (1u << 2)
#define PLAM_SEC_NOBITS   (1u << 3)
#define PLAM_SEC_RELOC    (1u << 4)
#define PLAM_SEC_DEBUG    (1u << 5)
#define PLAM_SEC_ENCRYPTED (1u << 6)
#define PLAM_SEC_PURGABLE  (1u << 7)

typedef struct {
    uint64_t name_off;
    uint32_t type;
    uint32_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint64_t entsize;
    uint64_t align;
    uint32_t section_prot;
    uint32_t comp_alg;
    uint32_t comp_level;
    uint32_t entropy;
    uint64_t hash_offset;
} plam_section_t;

/*-------------------------------- Relocations ---------------------------*/
#define PLAM_REL_NONE   0
#define PLAM_REL_64     1
#define PLAM_REL_ARM64  2
#define PLAM_REL_RISCV  3
#define PLAM_REL_ACCEL  4

typedef struct {
    uint64_t offset;
    uint32_t type;
    uint32_t sym_idx;
    int64_t  addend;
    uint32_t accelerator;
    uint32_t reserved;
} plam_reloc_t;

/*-------------------------------- Symbols --------------------------------*/
#define PLAM_SYM_NOTYPE  0
#define PLAM_SYM_FUNC    1
#define PLAM_SYM_OBJECT  2
#define PLAM_SYM_SECTION 3
#define PLAM_SYM_FILE    4
#define PLAM_SYM_COMMON  5
#define PLAM_SYM_TLS     6
#define PLAM_SYM_IFUNC   7
#define PLAM_SYM_ACCEL   8

#define PLAM_SYM_LOCAL   0
#define PLAM_SYM_GLOBAL  1
#define PLAM_SYM_WEAK    2

typedef struct {
    uint64_t name_off;
    uint64_t value;
    uint64_t size;
    uint8_t  type;
    uint8_t  bind;
    uint16_t section_idx;
    uint32_t version;
    uint32_t flags;
    uint32_t accelerator;
    uint32_t reserved;
} plam_symbol_t;

/*-------------------------------- Unwind --------------------------------*/
typedef struct {
    uint64_t begin_addr;
    uint64_t end_addr;
    uint64_t unwind_info_off;
    uint32_t flags;
    uint32_t personality_idx; /* Personality function symbol index */
    uint32_t accelerator;    /* Accelerator-specific unwind */
    uint32_t reserved;
} plam_unwind_entry_t;

/*-------------------------------- Dynamic linking -----------------------*/
#define PLAM_DEP_WEAK       (1u << 0)
#define PLAM_DEP_OPTIONAL   (1u << 1)
#define PLAM_DEP_REQUIRED   (1u << 2)
#define PLAM_DEP_REEXPORT   (1u << 3)

typedef struct {
    uint64_t name_off;
    uint64_t version_min;  /* Minimum required version */
    uint64_t version_max;  /* Maximum compatible version */
    uint8_t  uuid[16];     /* Library UUID */
    uint32_t flags;        /* PLAM_DEP_* */
    uint32_t compat_flags; /* Compatibility flags */
} plam_dependency_entry_t;

typedef struct {
    uint64_t name_off;
    uint64_t module_uuid[2]; /* Module identifier */
    uint64_t version;        /* Required version */
    uint32_t flags;
    uint32_t accelerator;    /* For accelerator-specific symbols */
} plam_import_entry_t;

/*-------------------------------- Compression ---------------------------*/
#define PLAM_COMP_NONE      0
#define PLAM_COMP_LZ4       1
#define PLAM_COMP_ZSTD      2
#define PLAM_COMP_LZMA      3
#define PLAM_COMP_BROTLI    4
#define PLAM_COMP_ZLIB      5

/* Compression levels */
#define PLAM_COMP_LEVEL_DEFAULT  0
#define PLAM_COMP_LEVEL_MIN      1
#define PLAM_COMP_LEVEL_MAX      22

/*-------------------------------- Global flags --------------------------*/
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
#define PLAM_F_RELOCS_STRIPPED (1u << 11)
#define PLAM_F_SMART_STACK    (1u << 12)
#define PLAM_F_LIVEPATCH      (1u << 13)
#define PLAM_F_MEMORY_COMPRESS (1u << 14)
#define PLAM_F_SECURE_LAUNCH   (1u << 15)
#define PLAM_F_CFI_ENABLED    (1u << 16)  /* Control Flow Integrity */
#define PLAM_F_SHADOW_STACK   (1u << 17)  /* Shadow Stack (CET) */
#define PLAM_F_MEM_TAGGING    (1u << 18)  /* ARM MTE / RISC-V J-extension */
#define PLAM_F_SEALED_HEAP    (1u << 19)  /* Sealed heap */
#define PLAM_F_PAC_ENABLED    (1u << 20)  /* ARM Pointer Authentication */
#define PLAM_F_PREFETCH_READY (1u << 21)  /* Prefetch optimized */
#define PLAM_F_LAZY_BINDING   (1u << 22)  /* Lazy binding */
#define PLAM_F_COMPRESSED_FILE   (1u << 23)
#define PLAM_F_ENCRYPTED_FILE    (1u << 24)

#define PLAM_RELRO_NONE 0
#define PLAM_RELRO_PART 1
#define PLAM_RELRO_FULL 2

/*-------------------------------- Subsystems ----------------------------*/
typedef enum : uint16_t {
    PLAM_SUBSYS_UNKNOWN       = 0,    /* Unknown subsystem */
    PLAM_SUBSYS_NATIVE_KERNEL = 1,    /* Native kernel components */
    PLAM_SUBSYS_DRIVER        = 2,    /* Device drivers */
    PLAM_SUBSYS_SYSTEM_SERV   = 3,    /* System services (daemons) */
    PLAM_SUBSYS_CONSOLE_APP   = 4,    /* Console applications */
    PLAM_SUBSYS_GUI_APP       = 5,    /* GUI applications */
    PLAM_SUBSYS_HYPERVISOR    = 6,    /* Hypervisor/virtualization */
    PLAM_SUBSYS_FIRMWARE      = 7,    /* Embedded firmware */
    PLAM_SUBSYS_SECURITY      = 8,    /* Security components */
    PLAM_SUBSYS_CONTAINER     = 9,    /* Containers */
    PLAM_SUBSYS_RUNTIME       = 10,   /* Execution environments (WASM, JVM) */
    PLAM_SUBSYS_RECOVERY      = 11,   /* Recovery mode */
    PLAM_SUBSYS_BOOTLOADER    = 12,   /* Bootloaders */
    PLAM_SUBSYS_WASM          = 13, 
    PLAM_SUBSYS_VENDOR_START  = 0x8000 /* Vendor-specific range start */
} plam_subsystem_t;

/* Subsystem flags */
#define PLAM_SUBSYS_F_REQUIRES_NETWORK  (1 << 0)
#define PLAM_SUBSYS_F_REQUIRES_STORAGE  (1 << 1)
#define PLAM_SUBSYS_F_REQUIRES_GPU      (1 << 2)
#define PLAM_SUBSYS_F_GUI_WINDOWED      (1 << 8)  /* Windowed mode support */
#define PLAM_SUBSYS_F_GUI_HIGH_DPI      (1 << 9)  /* HiDPI support */
#define PLAM_SUBSYS_F_DRIVER_HOTPLUG    (1 << 10) /* Hotplug support */
#define PLAM_SUBSYS_F_ISOLATED_EXEC     (1 << 11) /* Isolated execution */

/* NEW: Memory region flags for plam_mem_region_t */
#define PLAM_MEM_DMA         (1u << 0)  /* DMA-accessible */
#define PLAM_MEM_SECURE      (1u << 1)  /* Secure enclave */
#define PLAM_MEM_SHARED      (1u << 2)  /* Shared memory */
#define PLAM_MEM_NONCACHED   (1u << 3)  /* Non-cached */

/* NEW: Memory region structure (array via mem_regions RVA) */
typedef struct {
    uint64_t base;    /* Base address (preferred or required) */
    uint64_t size;    /* Size of region */
    uint32_t flags;   /* PLAM_MEM_* */
    uint32_t reserved;
} plam_mem_region_t;

/* Subsystem parameters */
typedef union {
    struct {  /* GUI applications */
        uint32_t min_width;
        uint32_t min_height;
        uint8_t  color_depth;
        uint8_t  dpi_aware;
        uint16_t gfx_requirements;
    } gui;
    
    struct {  /* Drivers */
        uint16_t device_class;
        uint16_t protocol_ver;
        uint32_t io_privileges;
    } driver;
    
    struct {  /* Containers */
        uint8_t  isolation_level;
        uint8_t  ns_flags;
        uint16_t cap_count;
    } container;
    
    struct {  /* Hypervisors */
        uint32_t vm_extensions;
        uint16_t max_vcpus;
        uint16_t max_ram_slots;
    } hypervisor;

     struct {  /* WASM */
        uint32_t wasm_memory_min;   /* Мин. страниц (64 KiB) */
        uint32_t wasm_memory_max;
        uint32_t stack_size;
        uint8_t  enable_simd;
        uint8_t  enable_threads;
        uint16_t reserved;
    } wasm;
    
    uint8_t raw[24];  /* Raw data for custom subsystems */
    uint64_t ext_off; /* NEW: Offset to TLV extensions for dynamic fields */
} plam_subsystem_params_t;

/* Execution environments */
#define PLAM_SUBSYS_ENV_KERNEL_SPACE  0x01
#define PLAM_SUBSYS_ENV_USER_SPACE    0x02
#define PLAM_SUBSYS_ENV_SECURE_ENC    0x04

/* Isolation levels */
#define PLAM_ISOL_NONE        0
#define PLAM_ISOL_USER        1
#define PLAM_ISOL_SANDBOX     2
#define PLAM_ISOL_CONTAINER   3
#define PLAM_ISOL_VM          4

/*-------------------------------- Directories table ---------------------*/
typedef struct {
    plam_rva_t security;         /* Digital signatures */
    plam_rva_t loadcfg;          /* Load configuration */
    plam_rva_t tls;              /* Thread-Local Storage */
    plam_rva_t cfg;              /* Configuration data */
    plam_rva_t exceptions;       /* Exception handling */
    plam_rva_t basereloc;        /* Base relocation table */
    plam_rva_t import_table;     /* Import table */
    plam_rva_t export_table;     /* Export table */
    plam_rva_t got;              /* Global Offset Table */
    plam_rva_t plt;              /* Procedure Linkage Table */
    plam_rva_t subsystem_validator; /* Subsystem validator */
    uint64_t   fat_off;          /* FAT multi-arch offset */
    uint32_t   fat_cnt;          /* FAT entry count */
    uint32_t   fat_flags;        /* FAT flags */
} plam_directories_t;

/*-------------------------------- Program headers ---------------------*/
typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
    uint8_t  accelerator;
    uint8_t  mem_space;
    uint16_t acc_flags;
    uint32_t acc_priv;
    uint32_t reserved_ph;
} plam_phdr_t;

/*-------------------------------- FAT arch entry ------------------------*/
#define PLAM_FAT_HAS_PHDR   (1u << 0)
#define PLAM_FAT_COMPRESSED (1u << 1)

#define PLAM_ACCEL_NONE  0x00
#define PLAM_ACCEL_GPU   0x01
#define PLAM_ACCEL_TPU   0x02
#define PLAM_ACCEL_FPGA  0x03
#define PLAM_ACCEL_NPU   0x04

typedef struct {
    uint16_t cpu_id;
    uint16_t cpu_sub;
    uint16_t abi_ver;
    uint16_t align_log2;
    uint64_t offset;
    uint64_t size;
    uint32_t flags;
    uint8_t  accelerator;
    uint8_t  mem_space;
    uint16_t vendor_id;
    uint64_t ph_off;
    uint16_t ph_count;
    uint16_t ph_entsize;
} plam_fatarch_t;
/*-------------------------------- Debug ------------------------*/
#define PLAM_DEBUG_DWARF  1
#define PLAM_DEBUG_PDB    2
#define PLAM_DEBUG_CUSTOM 3

typedef struct {
    plam_rva_t debug;
    uint16_t debug_type;
    uint16_t debug_version;
    uint32_t debug_size;
} plam_debug_info_t;

/*-------------------------------- Manifest ------------------------------*/
typedef struct {
    uint64_t min_os_ver;     /* Minimum OS version */
    uint64_t target_os_ver;  /* Target OS version */
    uint32_t feature_flags;  /* Required CPU features */
    uint32_t security_flags; /* Security requirements */
} plam_manifest_req_t;

/*-------------------------------- Build info --------------------------*/
typedef struct {
    uint64_t build_timestamp;
    uint64_t source_hash;
    uint32_t toolchain_ver;
    uint32_t optimization;
    char     builder_name[32];
    uint32_t build_flags;
    uint32_t reserved;
} plam_build_info_t;

/*-------------------------------- Main header ---------------------------*/
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint64_t flags;
    uint64_t file_size;
    uint32_t hdr_crc32;
    uint32_t file_crc32;

    uint32_t format_version;
    uint32_t content_version;

    uint64_t image_base;
    uint64_t entry_offset;
    uint64_t stack_reserve;
    uint64_t stack_commit;
    uint64_t heap_reserve;
    uint64_t heap_commit;

    uint16_t cpu_id;
    uint16_t cpu_sub;
    uint32_t abi_version;
    uint64_t cpu_features;
    uint32_t os_abi;
    uint32_t os_version_min;
    uint32_t os_version_sdk;

    plam_rva_t str_table;
    plam_rva_t sym_table;
    uint64_t section_table_off;
    uint32_t section_count;
    uint64_t reloc_table_off;
    uint32_t reloc_count;

    uint64_t ph_off;
    uint16_t ph_count;
    uint16_t ph_entry_size;

    plam_rva_t resources;
    plam_debug_info_t debug;

    uint8_t uuid[16];
    uint8_t build_hash[48];
    uint64_t timestamp;
    uint16_t crypto_mode;
    uint16_t hash_type;
    uint16_t sig_scheme;
    uint8_t  relro_level;
    uint8_t  file_compression;

    plam_rva_t manifest;
    uint32_t deps_count;
    uint32_t res_count;

    uint32_t lang_mask;
    uint16_t tool_major;
    uint16_t tool_minor;
    uint16_t tool_patch;
    uint16_t stdlib_ver;
    uint8_t  comp_model;
    uint8_t  lto_pgo_flags;
    uint8_t  opt_level;
    uint8_t  debug_level;

    plam_directories_t dirs;

    uint64_t control_flow_start;
    uint64_t control_flow_size;
    uint32_t hotpatch_offset;
    uint32_t hotpatch_count;

    uint16_t subsystem_type;
    uint16_t subsystem_version;
    plam_subsystem_params_t subsystem_params;
    uint32_t subsystem_flags;

    uint16_t sec_flags;
    uint8_t  sanitizer_level;
    uint8_t  crypt_alg;
    uint8_t  branch_prot;
    uint8_t  prefetch_hint;
    uint16_t cache_align;

    uint8_t  isolation_level;
    uint8_t  namespace_flags;
    uint16_t container_features;

    uint64_t metadata_size;
    plam_rva_t build_info;
    plam_rva_t api_constraints;

    uint64_t code_size;
    uint64_t init_data_size;
    uint64_t uninit_data_size;

    uint64_t ext_hdr_off;

    plam_rva_t mem_regions;  /* NEW: RVA to array of plam_mem_region_t for custom memory areas */

    uint8_t endian;          /* NEW: 0 = Little-Endian, 1 = Big-Endian */
    uint8_t  reserved[63];  /* Adjusted reserved (was 64) */
} plam_header_t;

/*-------------------------------- Extended manifest --------------------*/
typedef struct {
    plam_rva_t mods_dir;           /* Дочерние модули (для composite binaries) */
    plam_rva_t l10n_table;         /* Таблица локализации */
    plam_rva_t src_repo;           /* URL репозитория (для аудита) */
    uint32_t   abi_revision;       /* Ревизия ABI модуля */
    uint32_t   build_flags;        /* Флаги сборки (LTO, PGO, debug) */
    
    /* Системные требования */
    plam_manifest_req_t requirements; /* Мин. версия ОС, CPU features */
    
    uint32_t   min_kernel_api;     /* Минимальная версия API ядра */
    uint32_t   target_kernel_api;  /* Целевая версия API ядра */
    
    /* Права и возможности */
    uint64_t   required_caps[2];   /* Требуемые capability-биты (128) */
    uint32_t   api_level;          /* Уровень API (PlumOS SDK) */
    uint32_t   compat_flags;       /* Флаги совместимости (например, legacy syscalls) */

    uint32_t   compat_level;       /* NEW: Compatibility level (e.g., for legacy syscalls) */

    /* Зарезервировано */
    uint8_t    reserved[28];       /* Adjusted reserved (was 32) */
} plam_manifest_ext_t;

/*-------------------------------- Resource descriptor ------------------*/
typedef struct {
    uint32_t magic;              /* PLAM_RES_MAGIC */
    uint16_t type;               /* plam_res_type_t or vendor */
    uint16_t flags;              /* Resource flags */
    plam_rva_t blob;             /* Resource data */
    uint64_t orig_size;          /* Uncompressed size */
    uint8_t  comp_alg;           /* PLAM_COMP_* */
    uint8_t  comp_level;         /* Compression level */
    char     lang[6];            /* Localization (e.g. "en-US") */
    uint8_t  hash[48];           /* Integrity hash (BLAKE3-384) */
    uint8_t  reserved[2];
} plam_resource_t;

/*-------------------------------- Kernel module ------------------------*/
#define PLAM_KMOD_LIVEPATCH   (1u << 0)  /* Supports live patching */
#define PLAM_KMOD_SECURELOAD  (1u << 1)  /* Requires secure loading */
#define PLAM_KMOD_HOT_SWAP    (1u << 2)  /* Hot swapping */
#define PLAM_KMOD_SANDBOXED   (1u << 3)  /* Sandboxed execution */

typedef struct {
    uint64_t mod_base;           /* Module base address */
    uint64_t mod_size;           /* Module size */
    uint64_t init_fn;            /* Initialization function */
    uint64_t fini_fn;            /* Finalization function */
    uint32_t req_kernel_ver;     /* Required kernel version */
    uint32_t min_kernel_ver;     /* Minimum kernel version */
    uint32_t flags;              /* PLAM_KMOD_* flags */
    uint32_t dep_count;          /* Dependency count */
    uint64_t dep_offset;         /* Dependency UUID list offset */
} plam_kernelmod_t;

#pragma pack(pop)