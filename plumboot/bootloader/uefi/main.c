#include "efi.h"

EFI_STATUS EFI_CALL efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    const uint16_t *msg = u"\r\n[PlumBoot] Welcome to PlumOS!\r\n";
    SystemTable->ConOut->OutputString(SystemTable->ConOut, msg);
    return EFI_SUCCESS;
}