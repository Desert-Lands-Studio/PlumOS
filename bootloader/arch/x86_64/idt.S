.global idt_load

idt:
    .fill 256, 8, 0  

idt_ptr:
    .word idt_ptr - idt - 1
    .quad idt

idt_load:
    lidt [idt_ptr]
    ret