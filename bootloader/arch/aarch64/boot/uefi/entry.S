.global efi_main

efi_main:
    mov x19, x0  
    mov x20, x1  

    
    bl uefi_init

    mov x0, x19
    mov x1, x20
    bl rust_loader_entry

    
    b .

uefi_init:
    
    ret