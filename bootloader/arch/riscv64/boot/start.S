.section ".text.init"
.global _start
_start:
    la sp, __stack_top

    # Очистка BSS
    la a0, __bss_start
    la a1, __bss_end
    sub a1, a1, a0
    call memzero

    # Вызов Rust entry
    li a0, 0  # argc
    li a1, 0  # argv
    call bootloader_main

halt:
    wfi
    j halt

boot_message:
    .asciz "RISCV64 Minimal Bootloader Initialized!\n"

.section ".bss"
    .align 4
    .skip 0x4000
__stack_top: