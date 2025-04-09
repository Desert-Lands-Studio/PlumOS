#ifndef EFI_H
#define EFI_H

#include <stdint.h>

#define EFI_SUCCESS 0
#define EFIAPI __attribute__((ms_abi))
#define EFI_CALL EFIAPI

// Основные типы
typedef void* EFI_HANDLE;
typedef uint64_t EFI_STATUS;

typedef struct {
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t CRC32;
    uint32_t Reserved;
} EFI_TABLE_HEADER;

// Протокол вывода
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFI_CALL *EFI_TEXT_STRING)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*, const uint16_t*);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_STRING OutputString;
};

// Системная таблица UEFI
typedef struct {
    EFI_TABLE_HEADER Hdr;
    char _pad1[24]; // Упрощённо пропущены поля
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
} EFI_SYSTEM_TABLE;

#endif // EFI_H
