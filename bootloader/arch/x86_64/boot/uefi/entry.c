#include <efi.h>
#include <efilib.h>

extern EFI_STATUS efi_load_and_run_kernel(EFI_HANDLE, EFI_SYSTEM_TABLE*);

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"PlumOS Bootloader\n");
    return efi_load_and_run_kernel(ImageHandle, SystemTable);
}