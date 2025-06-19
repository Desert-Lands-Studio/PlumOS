[BITS 64]
[ORG 0x100000]

global _start
extern rust_main

_start:
    cli
    xor ax, ax
    mov ds, ax
    ; инициализация стека
    mov rsp, 0x80000
    call rust_main
    hlt
.loop:
    jmp .loop
