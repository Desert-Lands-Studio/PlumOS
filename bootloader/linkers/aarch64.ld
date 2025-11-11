ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS {
    . = 0x40000000;
    
    .text.boot : {
        *(.text.boot)
    } > RAM
    
    .text : {
        *(.text .text.*)
    } > RAM
    
    .rodata : {
        *(.rodata .rodata.*)
    } > RAM
    
    .data : {
        *(.data .data.*)
    } > RAM
    
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    } > RAM
    
    . = ALIGN(16);
    __stack_bottom = .;
    . = . + 0x10000;
    __stack_top = .;
    
    /DISCARD/ : {
        *(.comment)
        *(.note.*)
    }
}