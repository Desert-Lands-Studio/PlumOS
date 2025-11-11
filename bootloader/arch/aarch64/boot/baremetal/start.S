.section ".text.boot"
.global _start
.type _start, @function

_start:
    ldr x30, =__stack_top
    mov sp, x30

    ldr x0, =__bss_start
    ldr x1, =__bss_end
    cmp x1, x0
    beq 2f
1:
    str xzr, [x0], #8
    cmp x0, x1
    blt 1b
2:
    bl _start_rust

halt:
    wfi
    b halt