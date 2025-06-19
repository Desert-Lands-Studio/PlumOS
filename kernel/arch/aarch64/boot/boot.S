/* Minimal AArch64 boot stub */
.globl _start
.type _start, %function
.extern rust_main

_start:
    /* Disable interrupts */
    msr daifset, #0xf
    /* Simple stack at 0x80000 */
    ldr x0, =_stack_top
    mov sp, x0
    bl rust_main
1:
    wfi
    b 1b

.section .bss
.align 12
_stack_bottom:
.space 0x10000
_stack_top:
