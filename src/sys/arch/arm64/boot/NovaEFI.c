#include <Uefi.h>
#include <UefiLib.h>
#include <UefiBootServicesTableLib.h>
#include <Base.h>

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
) {
    Print(L"Hello, PlumOS UEFI World!\n");

    UINT32 eax, ebx, ecx, edx;

    __asm__ __volatile__ (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0)
    );
    Print(L"CPUID: EAX=%x, EBX=%x, ECX=%x, EDX=%x\n", eax, ebx, ecx, edx);

    return EFI_SUCCESS;
}
