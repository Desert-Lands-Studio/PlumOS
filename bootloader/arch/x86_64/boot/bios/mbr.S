.code16
.intel_syntax noprefix

.global start
start:
    jmp short start_stage2

.balign 512, 0x0

start_stage2:
    cli 
    cld 

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov ah, 0x0e
    mov bh, 0x00
    mov bl, 0x07

    mov si, offset msg
print_char:
    lodsb
    or al, al
    jz end_boot
    int 0x10
    jmp print_char

end_boot:
    jmp $

msg:
   .ascii "x86_64 BIOS MBR Bootloader Initialized!\n"
   .byte 0

.org 510
.word 0xaa55