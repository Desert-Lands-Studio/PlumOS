#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "include/plam_format.h"

// Максимальный размер буфера для ресурсов
#define MAX_BUFFER_SIZE (1024 * 1024) // 1 MB

// Структура для конфигурации
typedef struct {
    char target[16];        // "app" или "kernel"
    char arch[16];          // "x86_64" или "arm64"
    char compression[16];   // "none", "lz4", "lzma", "zstd"
    char resources[64];     // Список ресурсов: "icon,locale,signature"
    int encrypt;            // 1 = шифровать, 0 = нет
    int secure_boot;        // 1 = Secure Boot, 0 = нет
} packer_config_t;

// Простая реализация LZ4 (упрощённая для примера)
uint32_t lz4_compress(uint8_t* input, uint32_t in_size, uint8_t* output) {
    // Упрощённая версия: копируем данные без реального сжатия
    // В реальной реализации используйте словарь и повторяющиеся последовательности
    memcpy(output, input, in_size);
    return in_size;
}

// Простая реализация LZMA (заглушка для примера)
uint32_t lzma_compress(uint8_t* input, uint32_t in_size, uint8_t* output) {
    // Для реальной LZMA нужен Lempel-Ziv + арифметическое кодирование
    memcpy(output, input, in_size);
    return in_size;
}

// Простая реализация ZSTD (заглушка для примера)
uint32_t zstd_compress(uint8_t* input, uint32_t in_size, uint8_t* output) {
    // Для реальной ZSTD нужен блочный подход с энтропийным кодированием
    memcpy(output, input, in_size);
    return in_size;
}

// Универсальная функция сжатия
uint32_t compress_data(uint8_t* input, uint32_t in_size, uint8_t* output, uint16_t comp_type) {
    switch (comp_type) {
        case PLAM_COMP_LZ4:   return lz4_compress(input, in_size, output);
        case PLAM_COMP_LZMA:  return lzma_compress(input, in_size, output);
        case PLAM_COMP_ZSTD:  return zstd_compress(input, in_size, output);
        case PLAM_COMP_NONE:
        default:              memcpy(output, input, in_size); return in_size;
    }
}

// Вычисление CRC32
uint32_t calculate_crc32(uint8_t* data, uint64_t size) {
    uint32_t crc = 0xFFFFFFFF;
    static const uint32_t crc_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };

    for (uint64_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[i] >> 0)) & 0x0F];
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[i] >> 4)) & 0x0F];
    }
    return ~crc;
}

// Генерация UUID (простая версия на основе времени и случайных чисел)
void generate_uuid(uint8_t* uuid) {
    // Реальная реализация должна использовать системное время или аппаратный RNG
    static uint32_t seed = 123456789;
    for (int i = 0; i < 16; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        uuid[i] = (uint8_t)(seed >> (i % 24));
    }
}

// Вычисление SHA-256 (упрощённая реализация)
void calculate_sha256(uint8_t* data, uint64_t size, uint8_t* hash) {
    // Инициализация констант SHA-256
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    // Упрощённая обработка: реальная версия требует пэдинг и 64-байтовые блоки
    for (uint64_t i = 0; i < size; i += 4) {
        uint32_t chunk = (i < size) ? data[i] : 0;
        for (int j = 0; j < 8; j++) {
            h[j] ^= chunk;
        }
    }
    // Заполняем хеш
    for (int i = 0; i < 8; i++) {
        hash[i*4 + 0] = (h[i] >> 24) & 0xFF;
        hash[i*4 + 1] = (h[i] >> 16) & 0xFF;
        hash[i*4 + 2] = (h[i] >> 8) & 0xFF;
        hash[i*4 + 3] = h[i] & 0xFF;
    }
}

// Чтение конфигурационного файла
int read_config(const char* config_file, packer_config_t* config) {
    FILE* fp = fopen(config_file, "r");
    if (!fp) {
        printf("Cannot open config file: %s\n", config_file);
        return 0;
    }

    char line[128];
    memset(config, 0, sizeof(packer_config_t));
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "target=", 7) == 0) {
            sscanf(line, "target=%s", config->target);
        } else if (strncmp(line, "arch=", 5) == 0) {
            sscanf(line, "arch=%s", config->arch);
        } else if (strncmp(line, "compression=", 11) == 0) {
            sscanf(line, "compression=%s", config->compression);
        } else if (strncmp(line, "resources=", 10) == 0) {
            sscanf(line, "resources=%s", config->resources);
        } else if (strncmp(line, "encrypt=", 8) == 0) {
            char val[4];
            sscanf(line, "encrypt=%s", val);
            config->encrypt = (strcmp(val, "yes") == 0) ? 1 : 0;
        } else if (strncmp(line, "secure_boot=", 12) == 0) {
            char val[4];
            sscanf(line, "secure_boot=%s", val);
            config->secure_boot = (strcmp(val, "yes") == 0) ? 1 : 0;
        }
    }
    fclose(fp);
    return 1;
}

// Преобразование строки ресурса в plam_res_type_t
plam_res_type_t parse_resource_type(const char* res) {
    if (strcmp(res, "icon") == 0) return PLAM_RES_ICON;
    if (strcmp(res, "locale") == 0) return PLAM_RES_LOCALE;
    if (strcmp(res, "security") == 0) return PLAM_RES_SECURITY;
    if (strcmp(res, "metadata") == 0) return PLAM_RES_METADATA;
    if (strcmp(res, "manifest") == 0) return PLAM_RES_MANIFEST;
    if (strcmp(res, "signature") == 0) return PLAM_RES_SIGNATURE;
    if (strcmp(res, "license") == 0) return PLAM_RES_LICENSE;
    return 0;
}

// Основная функция пакера
int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <config> <code.bin> <output.plam>\n", argv[0]);
        return 1;
    }

    // Чтение аргументов
    const char* config_file = argv[1];
    const char* code_file = argv[2];
    const char* output_file = argv[3];

    // Чтение конфигурации
    packer_config_t config;
    if (!read_config(config_file, &config)) {
        return 1;
    }

    // Определение архитектуры
    uint32_t target_arch = PLAM_ARCH_X86_64;
    if (strcmp(config.arch, "arm64") == 0) {
        target_arch = PLAM_ARCH_ARM64;
    }

    // Определение сжатия
    uint16_t comp_type = PLAM_COMP_NONE;
    if (strcmp(config.compression, "lz4") == 0) comp_type = PLAM_COMP_LZ4;
    else if (strcmp(config.compression, "lzma") == 0) comp_type = PLAM_COMP_LZMA;
    else if (strcmp(config.compression, "zstd") == 0) comp_type = PLAM_COMP_ZSTD;

    // Чтение кода
    FILE* code_fp = fopen(code_file, "rb");
    if (!code_fp) {
        printf("Cannot open code file: %s\n", code_file);
        return 1;
    }
    fseek(code_fp, 0, SEEK_END);
    uint64_t code_size = ftell(code_fp);
    fseek(code_fp, 0, SEEK_SET);
    uint8_t* code_data = malloc(code_size);
    if (!code_data) {
        printf("Memory allocation failed\n");
        fclose(code_fp);
        return 1;
    }
    fread(code_data, 1, code_size, code_fp);
    fclose(code_fp);

    // Подготовка ресурсов (пример: иконка и подпись)
    plam_resource_t resources[16] = {0};
    uint32_t resource_count = 0;
    uint64_t resources_size = 0;

    // Парсинг ресурсов из конфигурации
    char res_list[64];
    strcpy(res_list, config.resources);
    char* token = strtok(res_list, ",");
    while (token && resource_count < 16) {
        plam_res_type_t res_type = parse_resource_type(token);
        if (res_type == 0) {
            printf("Unknown resource: %s\n", token);
            free(code_data);
            return 1;
        }

        plam_resource_t* res = &resources[resource_count];
        res->magic = PLAM_RES_MAGIC;
        res->type = res_type;
        res->size = 1024; // Пример: фиксированный размер ресурса
        res->compression_type = comp_type;
        res->flags = config.encrypt ? PLAM_RESFLAG_ENCRYPTED : PLAM_RESFLAG_NONE;

        // Заглушка: данные ресурса
        uint8_t* res_data = malloc(res->size);
        memset(res_data, res_type, res->size); // Заполняем типом для примера
        uint8_t* comp_data = malloc(res->size);
        res->compressed_size = compress_data(res_data, res->size, comp_data, comp_type);
        calculate_sha256(res_data, res->size, res->hash);

        res->offset = sizeof(plam_header_t) + code_size + resources_size;
        resources_size += res->compressed_size;
        resource_count++;

        free(res_data);
        free(comp_data);
        token = strtok(NULL, ",");
    }

    // Создание заголовка PLAM
    plam_header_t header = {0};
    header.magic = PLAM_MAGIC;
    header.version_major = 1;
    header.version_minor = 0;
    header.entry_point = 0x1000; // Пример: точка входа
    header.file_flags = config.secure_boot ? (1 << 1) : 0; // Пример флага Secure Boot
    header.target_arch = target_arch;
    header.code_offset = sizeof(plam_header_t);
    header.code_size = code_size;
    header.data_offset = 0; // Пока без данных
    header.data_size = 0;
    header.resources_offset = header.code_offset + code_size;
    header.resource_count = resource_count;
    generate_uuid(header.uuid);
    header.checksum = calculate_crc32(code_data, code_size);
    strncpy(header.version_tag, "v1.0.0", 8);

    // Запись выходного файла
    FILE* out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        printf("Cannot open output file: %s\n", output_file);
        free(code_data);
        return 1;
    }

    // Записываем заголовок
    fwrite(&header, sizeof(plam_header_t), 1, out_fp);

    // Записываем код
    fwrite(code_data, code_size, 1, out_fp);

    // Записываем ресурсы
    for (uint32_t i = 0; i < resource_count; i++) {
        // Заглушка: данные ресурса
        uint8_t* res_data = malloc(resources[i].size);
        memset(res_data, resources[i].type, resources[i].size);
        uint8_t* comp_data = malloc(resources[i].size);
        uint32_t comp_size = compress_data(res_data, resources[i].size, comp_data, comp_type);
        fwrite(comp_data, comp_size, 1, out_fp);
        free(res_data);
        free(comp_data);
    }

    // Записываем метаданные ресурсов
    fseek(out_fp, header.resources_offset, SEEK_SET);
    fwrite(resources, sizeof(plam_resource_t), resource_count, out_fp);

    fclose(out_fp);
    free(code_data);

    printf("PLAM file created: %s\n", output_file);
    return 0;
}