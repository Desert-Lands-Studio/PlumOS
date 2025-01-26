#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
) {
    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;

    Print(L"PlumLoader: UEFI Application Started\n");

    Status = gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        (VOID **)&GraphicsOutput
    );

    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate Graphics Output Protocol: %r\n", Status);
        return Status;
    }

    if (GraphicsOutput->Mode == NULL) {
        Print(L"GraphicsOutput->Mode is NULL. Graphics may not be initialized.\n");
        return EFI_UNSUPPORTED;
    }

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL BackgroundColor = { 24, 5, 22, 0 }

    Status = GraphicsOutput->Blt(
        GraphicsOutput,
        &BackgroundColor,
        EfiBltVideoFill,
        0, 0,
        0, 0,
        GraphicsOutput->Mode->Info->HorizontalResolution,
        GraphicsOutput->Mode->Info->VerticalResolution,
        0
    );

    if (EFI_ERROR(Status)) {
        Print(L"Failed to set background color: %r\n", Status);
        return Status;
    }

    Print(L"Background color set to blue successfully.\n");

    Print(L"Press any key to exit.\n");
    SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);

    while (TRUE) {
        EFI_INPUT_KEY Key;
        Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
        if (!EFI_ERROR(Status)) {
            Print(L"Exiting PlumLoader.\n");
            break;
        }
    }

    return EFI_SUCCESS;
}
