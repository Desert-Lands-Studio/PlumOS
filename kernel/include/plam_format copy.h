#pragma once
#include <stdint.h>
 
#pragma pack(push,1) 

/*-------------------------------- Magic ---------------------------------*/
#define PLAM_MAGIC        0x504C414Du  /* "PLAM" */
#define PLAM_RES_MAGIC    0x504C4D52u  /* "PLMR" */
#define PLAM_FAT_MAGIC    0x504C4D46u  /* "PLMF" */
 
typedef struct { uint64_t off, sz; } plam_rva_t;
/*-------------------------------- CPU Architecture -------------------------*/
typedef enum : uint16_t {
     PLAM_CPU_NONE     = 0x0000,
     PLAM_CPU_X86_64   = 0x8664,   /* EM_X86_64  */
     PLAM_CPU_ARM64    = 0xAA64,   /* EM_AARCH64 */
     PLAM_CPU_RISCV64  = 0xF3,     /* EM_RISCV   */
     PLAM_CPU_UNKNOWN  = 0xFFFF
} plam_cpu_t;
 
 /* Sub‑architecture / feature sub‑type bits */
typedef enum : uint16_t {
     PLAM_CPU_X86_64_SSE4   = 0x0001,
     PLAM_CPU_ARM64_NEON    = 0x0100,
     PLAM_CPU_RISCV_VECTOR  = 0x1000
} plam_cpu_subtype_t;
 
 /*-------------------------------- File Types -------------------------------*/
typedef enum : uint16_t {
     PLAM_FT_NONE      = 0x00,
     PLAM_FT_KERNEL    = 0x01,
     PLAM_FT_DRIVER    = 0x02,
     PLAM_FT_SHARED    = 0x03,  /* динамическая библиотека  */
     PLAM_FT_APP       = 0x04,
     PLAM_FT_MODULE    = 0x05,  /* hot‑plug kernel/user     */
     PLAM_FT_BOOT      = 0x06,
     PLAM_FT_PLUGIN    = 0x07,
     PLAM_FT_OBJECT    = 0x08   /* relocatable .o           */
} plam_file_type_t;
 
 /*-------------------------------- Security ---------------------------------*/
 /* Signature algorithms (extensible) */
#define PLAM_SIG_ED25519   1
#define PLAM_SIG_ECDSA     2
#define PLAM_SIG_QUANTUM   3
 
typedef struct {
     uint8_t sig_type;        // 1=Ed25519, 2=ECDSA, 3=QuantumSafe
     uint8_t key_revocation;  // Механизм отзыва ключей
     uint16_t cert_count;     // Количество сертификатов в цепочке
     uint64_t timestamp;      // Время подписания (ns since epoch)
     uint8_t reserved[16];
} plam_sig_header_t;
 
 /*-------------------------------- Resource Types ---------------------------*/
typedef enum : uint16_t {
     PLAM_RES_ICON        = 0x0100,  /* Иконка с plam_icon_info_t       */
     PLAM_RES_VERSION     = 0x0200,  /* Версия в семантическом формате  */
     PLAM_RES_DEPENDENCY  = 0x0300,  /* Зависимости в формате CBOR      */
     PLAM_RES_STRING      = 0x0400,  /* Локализованные строки           */
     PLAM_RES_UI          = 0x0500,  /* GUI-ресурсы (макеты, шрифты)    */
     PLAM_RES_VENDOR      = 0xF000   /* Произвольные данные вендора     */
} plam_res_type_t;
 
 /*------------------------------------------------------------------------*
  *  Icon Resource Specification                                          *
  *------------------------------------------------------------------------*/
typedef struct {
     uint32_t width;          /* в пикселях */
     uint32_t height;
     uint8_t  format;         /* 0=RGBA32, 1=BC7, 2=PVRTC */
     uint8_t  mip_levels;     /* количество MIP-уровней */
     uint16_t reserved;
} plam_icon_info_t;
 
/*------------------------------------------------------------------------*
 *  Section Flags (permissions and attributes)                            *
 *------------------------------------------------------------------------*/
 #define PLAM_SEC_READ    (1u << 0)     // Чтение
 #define PLAM_SEC_WRITE   (1u << 1)     // Запись
 #define PLAM_SEC_EXEC    (1u << 2)     // Исполнение
 #define PLAM_SEC_NOBITS  (1u << 3)     // Секция без данных (как .bss)
 #define PLAM_SEC_RELOC   (1u << 4)     // Содержит релокации

/*------------------------------------------------------------------------*
 *  Section Header                                                        *
 *------------------------------------------------------------------------*/
 typedef struct {
    uint64_t name_off;             // Смещение имени секции в таблице строк
    uint32_t type;                 // Тип секции (код, данные и т.д.)
    uint32_t flags;                // Флаги (PLAM_SEC_*)
    uint64_t addr;                 // Виртуальный адрес загрузки
    uint64_t offset;               // Смещение данных в файле
    uint64_t size;                 // Размер секции в файле
    uint64_t entsize;              // Размер записи (для таблиц)
    uint32_t comp_alg;             // Алгоритм сжатия (PLAM_COMP_*)
    uint32_t reserved;             // Зарезервировано
} plam_section_t;

/*------------------------------------------------------------------------*
 *  Relocation Types (example for x86_64)                                 *
 *------------------------------------------------------------------------*/
#define PLAM_REL_NONE    0             // Нет релокации
#define PLAM_REL_64      1             // 64-битная абсолютная релокация
#define PLAM_REL_PC32    2             // 32-битная PC-относительная релокация

/*------------------------------------------------------------------------*
 *  Relocation Entry                                                      *
 *------------------------------------------------------------------------*/
typedef struct {
    uint64_t offset;               // Смещение в секции для релокации
    uint32_t type;                 // Тип релокации (PLAM_REL_*)
    uint32_t sym_idx;              // Индекс в таблице символов
    int64_t  addend;               // Добавка для релокации
} plam_reloc_t;

/*------------------------------------------------------------------------*
 *  Symbol Table Entry                                                    *
 *------------------------------------------------------------------------*/
typedef struct {
    uint64_t name_off;             // Смещение имени символа
    uint64_t value;                // Значение символа (адрес/смещение)
    uint64_t size;                 // Размер символа
    uint8_t  type;                 // Тип символа (функция, объект)
    uint8_t  bind;                 // Привязка (локальный, глобальный)
    uint16_t section_idx;          // Индекс секции символа
    uint32_t version;              // Версия символа
} plam_symbol_t;

/*------------------------------------------------------------------------*
 *  Unwind Information                                                    *
 *------------------------------------------------------------------------*/
typedef struct {
    uint64_t begin_addr;           // Начальный адрес функции
    uint64_t end_addr;             // Конечный адрес функции
    uint64_t unwind_info_off;      // Смещение данных unwind
    uint32_t flags;                // Дополнительные флаги
} plam_unwind_entry_t;

/*------------------------------------------------------------------------*
*  Dynamic Linking Enhancements                                          *
*------------------------------------------------------------------------*/
#define PLAM_DEP_WEAK        (1u << 0)
#define PLAM_DEP_OPTIONAL    (1u << 1)
  
typedef struct {
      uint64_t name_off;    /* offset inside string table */
      uint64_t version;     /* semantic version           */
      uint8_t  uuid[16];
      uint32_t flags;       /* combination of PLAM_DEP_*  */
} plam_dependency_entry_t;
 
/*------------------------------------------------------------------------*
*  Global flags (DLLCharacteristics / e_flags analogue)                  *
*------------------------------------------------------------------------*/
#define PLAM_F_PIE            (1u << 0)   /* позиционно‑независимый код          */
#define PLAM_F_ASLR           (1u << 1)   /* addr rand; отключаемо для отладки   */
#define PLAM_F_NX_STACK       (1u << 2)
#define PLAM_F_NX_HEAP        (1u << 3)
#define PLAM_F_GUARD_CF       (1u << 4)
#define PLAM_F_SEH_SAFE       (1u << 5)
#define PLAM_F_ISOLATED_MEM   (1u << 6)   /* HW изолятор / S-EL0 / VM‑ctx        */
#define PLAM_F_DEBUG_STRIPPED (1u << 7)
#define PLAM_F_NO_REEXPORTS   (1u << 8)
#define PLAM_F_HW_ACCEL       (1u << 9)
#define PLAM_F_HOT_PATCHABLE  (1u << 10)
 
#define PLAM_RELRO_NONE   0
#define PLAM_RELRO_PART   1
#define PLAM_RELRO_FULL   2 

/*------------------------------------------------------------------------*
 *  Compression algorithms                                                *
 *------------------------------------------------------------------------*/
#define PLAM_COMP_NONE   0
#define PLAM_COMP_LZ4    1
#define PLAM_COMP_ZSTD   2
#define PLAM_COMP_LZMA   3
#define PLAM_COMP_BROTLI 4    /* только для ресурсов ≤16 MiB */

/*------------------------------------------------------------------------*
 *  Global Flags                                                          *
 *------------------------------------------------------------------------*/
 #define PLAM_F_PIE            (1u << 0) // Позиционно-независимый код
 #define PLAM_F_ASLR           (1u << 1) // Рандомизация адресов
 #define PLAM_F_NX_STACK       (1u << 2) // Стек неисполняемый
 #define PLAM_F_DEBUG_STRIPPED (1u << 7) // Отладочная информация удалена

/*------------------------------------------------------------------------*
 *  “data‑directory” table (PE style)                                     *
 *------------------------------------------------------------------------*/
typedef struct {
    plam_rva_t security;   /* PKCS#7 / Ed25519 signature blob   */
    plam_rva_t loadcfg;    /* GuardCF / CET tables              */
    plam_rva_t tls;        /* TLS template                      */
    plam_rva_t cfg;        /* user config (TOML / CBOR)         */
    uint64_t   fat_off;    /* offset to FAT header (if any)     */
    uint32_t   fat_cnt;    /* number of architectures           */
    uint32_t   reserved;
} plam_directories_t;

 
 /*------------------------------------------------------------------------*
  *  FAT header entry (multi‑arch, Mach‑O style)                           *
  *------------------------------------------------------------------------*/
typedef struct {
    uint16_t cpu_id;       /* plam_cpu_t                       */
    uint16_t abi_ver;      /* toolchain ABI version            */
    uint32_t align_log2;   /* file alignment = 2^n             */
    uint64_t offset;       /* file offset of embedded image    */
    uint64_t size;         /* size of embedded image           */
} plam_fatarch_t;
 
 /*------------------------------------------------------------------------*
  *  Main PLAM header (on‑disk)                                            *
  *------------------------------------------------------------------------*/
typedef struct {
     /* ── file signature & format ────────────────────────────────────── */
     uint32_t magic;                 /* PLAM_MAGIC                     */
     uint16_t hdr_ver_major;         /* == 2 , bump if breaking        */
     uint16_t hdr_ver_minor;         /* compat additions               */
     uint16_t file_type;             /* plam_file_type_t               */
     uint16_t hdr_size;              /* sizeof(plam_header_t)          */
     uint32_t flags;                 /* PLAM_F_*                       */
     uint32_t hdr_crc32;             /* CRC32 over hdr_size, crc=0     */
 
     /* ── entry & architecture ───────────────────────────────────────── */
     uint64_t entry_off;             /* RVA → start (0 if none)        */
     uint16_t cpu_id;                /* plam_cpu_t (e_machine)         */
     uint16_t cpu_sub;               /* uArch / SIMD ext.              */
     uint32_t abi_ver;               /* libc / sys ABI                 */
     uint64_t cpu_feat;              /* CPUID feature mask             */
     
 /* ── string / symbol tables ─────────────────────────────────────── */
 plam_rva_t str_table;           /* string table (UTF‑8)            */
 plam_rva_t sym_table;           /* ELF‑style symbol table          */

 /* ── extended metadata ──────────────────────────────────────────── */
 plam_rva_t author_info;         /* UTF‑8 author/packager string    */
 plam_rva_t description;         /* UTF‑8 short description         */
 uint32_t   default_icon_idx;    /* index inside resource table     */

 /* ── fixed sections (RVA relative) ──────────────────────────────── */
 plam_rva_t code;                /* .text                           */
 plam_rva_t data;                /* .data (initialised)             */
 uint64_t   bss_size;            /* .bss (NOBITS)                   */
 uint64_t   stack_size;          /* default user stack              */
 uint64_t   import_off;          /* RVA of import table             */
 uint64_t   export_off;          /* RVA of export table             */
 plam_rva_t resources;           /* plam_resource_t[]               */
 plam_rva_t debug;               /* DWARF / CodeView stream         */

 /* ── identity / reproducibility ─────────────────────────────────── */
 uint8_t  uuid[16];              /* build UUID                      */
 uint8_t  build_hash[32];        /* BLAKE3‑256 over inputs          */
 uint64_t timestamp;             /* Unix epoch ms                   */
 uint32_t build_num;             /* CI counter / git height         */
 uint32_t reserved0;

 /* ── security / ABI level ───────────────────────────────────────── */
 uint32_t os_abi;                /* PlumOS=0, POSIX=3, …            */
 uint32_t os_ver_min;            /* e.g. 0x00020001 → 2.1           */
 uint32_t os_ver_sdk;            /* SDK/target version              */
 uint16_t crypto_mode;           /* 0=none/1=RSA/2=EdDSA            */
 uint16_t hash_type;             /* 1=SHA‑256/2=BLAKE3‑256 …        */
 uint16_t sig_scheme;            /* X509, CMS, minisign …           */
 uint8_t  relro_lvl;             /* PLAM_RELRO_*                    */
 uint8_t  file_comp;             /* PLAM_COMP_* (whole‑file)        */

 /* ── manifest dir (deps/resources) ──────────────────────────────── */
 plam_rva_t manifest;            /* CBOR manifest                   */
 uint32_t   deps_cnt;            /* number of deps                  */
 uint32_t   res_cnt;             /* number of resources             */

 /* ── toolchain / build info ─────────────────────────────────────── */
 uint32_t lang_mask;             /* PLAM_LANG_*                     */
 uint16_t tool_major, tool_minor;
 uint16_t stdlib_ver;
 uint8_t  comp_model;            /* 0=static 1=PIC 2=PIE            */
 uint8_t  lto_pgo_flags;         /* bit‑mask                        */

 
     /* ── data directories table ─────────────────────────────────────── */
     plam_directories_t dirs;
 
     uint8_t  reserved[16];          /* future‑use (align 8)           */
} plam_header_t;
 
 /*------------------------------------------------------------------------*
  *  Extended manifest (optional, referenced from dirs.manifest)           *
  *------------------------------------------------------------------------*/
typedef struct {
    plam_rva_t mods_dir;            /* /mods hot‑plug catalogue        */
    plam_rva_t l10n_table;          /* language → resource idx         */
    plam_rva_t src_repo;            /* serialized VCS metadata         */
    uint32_t   abi_rev;
    uint32_t   build_flags;         /* extra (LTO, PGO …)              */
} plam_manifest_ext_t;
 
/*------------------------------------------------------------------------*
 *  Resource descriptor (embedded in .rsrc)                                *
 *------------------------------------------------------------------------*/
typedef struct {
    uint32_t magic;                 /* PLAM_RES_MAGIC                  */
    uint16_t type;                  /* user‑defined                    */
    uint16_t flags;                 /* readonly/encrypted …            */
    plam_rva_t blob;                /* compressed data chunk           */
    uint64_t orig_size;
    uint8_t  comp_alg;              /* PLAM_COMP_*                     */
    char     lang[6];               /* e.g. "en-US"                   */
    uint8_t  hash[48];              /* BLAKE3‑384                      */
    uint8_t  extra[6];
} plam_resource_t;

/*------------------------------------------------------------------------*
 *  Kernel hot‑plug module table                                           *
 *------------------------------------------------------------------------*/
typedef struct {
    uint64_t mod_base, mod_size;    /* mapped address & size           */
    uint64_t init_fn, fini_fn;      /* entry points                    */
    uint32_t req_kernel_ver;        /* minimal running kernel build    */
    uint32_t flags;                 /* bit0 = LIVEPATCH                */
} plam_kernelmod_t;

#pragma pack(pop)
/*------------------------------- End ------------------------------------*/
