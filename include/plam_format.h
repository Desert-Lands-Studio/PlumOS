#ifndef PLAM_FORMAT_H
#define PLAM_FORMAT_H

#include <stdint.h>

#define PLAM_MAGIC       0x504C4D32u  // "PLM2"
#define PLAM_RES_MAGIC   0x504C4D32u  // "PLMR"

#define PLAM_ARCH_X86_64 (0x8664u)
#define PLAM_ARCH_ARM64  (0xAA64u)

typedef enum {
    PLAM_RES_ICON       = 0x01,
    PLAM_RES_LOCALE     = 0x02,
    PLAM_RES_SECURITY   = 0x03,
    PLAM_RES_METADATA   = 0x04,
    PLAM_RES_MANIFEST   = 0x05,
    PLAM_RES_SIGNATURE  = 0x06,
    PLAM_RES_LICENSE    = 0x07,
} plam_res_type_t;

#define PLAM_COMP_NONE   0
#define PLAM_COMP_LZ4    1
#define PLAM_COMP_LZMA   2
#define PLAM_COMP_ZSTD   3

#define PLAM_RESFLAG_NONE      0
#define PLAM_RESFLAG_ENCRYPTED (1 << 0)

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;              // PLAM_MAGIC = 'PLM2'
    uint16_t version_major;      // Мажорная версия формата
    uint16_t version_minor;      // Минорная версия
    uint64_t entry_point;        // Точка входа в код
    uint32_t file_flags;         // Флаги всего файла (например: debug, signed и т.д.)
    uint32_t target_arch;        // Архитектура: x86_64, arm64 и т.д.
    uint64_t code_offset;        // Смещение кода
    uint64_t code_size;          // Размер кода
    uint64_t data_offset;        // Смещение данных
    uint64_t data_size;          // Размер данных
    uint64_t resources_offset;   // Смещение секции ресурсов
    uint32_t resource_count;     // Кол-во ресурсов
    uint8_t  uuid[16];           // Уникальный идентификатор файла
    uint32_t checksum;           // Контрольная сумма (например, CRC32)
    char     version_tag[8];     // Версия (например "v1.0.0")
    uint8_t  reserved[24];       // Зарезервировано под будущее
} plam_header_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;              // PLAM_RES_MAGIC = 'PLMR'
    uint32_t type;               // Тип ресурса (plam_res_type_t)
    uint64_t offset;             // Смещение ресурса от начала файла
    uint64_t size;               // Размер после распаковки
    uint32_t compressed_size;    // Размер сжатого ресурса
    uint16_t compression_type;   // Тип сжатия (PLAM_COMP_*)
    uint32_t flags;              // Флаги ресурса (шифрован, критичный и т.д.)
    uint8_t  hash[32];           // Хеш SHA-256 ресурса
} plam_resource_t;
#pragma pack(pop)

#endif /* PLAM_FORMAT_H */