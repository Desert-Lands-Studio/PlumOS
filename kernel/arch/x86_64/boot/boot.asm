[BITS 64]
[ORG 0x100000]

global _start
_start:
    cli
    xor ax, ax
    mov ds, ax
    ; инициализация стека
    mov rsp, 0x80000
    call kmain
    hlt
.loop:
    jmp .loop
