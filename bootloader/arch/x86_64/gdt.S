.global gdt_load

gdt:
    .quad 0x0000000000000000  
    .quad 0x00af9b000000ffff  
    .quad 0x00af93000000ffff  

gdt_ptr:
    .word gdt_ptr - gdt - 1
    .quad gdt

gdt_load:
    lgdt [gdt_ptr]
    ret