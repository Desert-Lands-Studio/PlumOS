#ifndef PLM_FORMAT_H
#define PLM_FORMAT_H

#include <stdint.h>

#define PLM_MAGIC 0x504C4D32  // "PLM2" в ASCII

// Заголовок PLM-файла
typedef struct {
    uint32_t magic;         // Магическое число
    uint32_t version;       // Версия формата
    uint64_t entry_point;   // Точка входа программы
    uint64_t code_size;     // Размер кода
    uint64_t data_size;     // Размер данных
    uint64_t resources_offset; // Смещение секции ресурсов
    uint32_t resource_count;   // Количество ресурсов
} plm_header_t;

// Структура ресурса
typedef struct {
    uint32_t type;          // Тип ресурса (0x01 = изображение, 0x02 = иконка)
    uint32_t flags;         // Флаги (например, сжатие)
    uint64_t size;          // Размер ресурса
    uint64_t offset;        // Смещение ресурса
} plm_resource_t;

#endif // PLM_FORMAT_H
