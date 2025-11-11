#include <stdint.h>

void print_char(char c) {
    asm volatile (
        "mov $0x0e, %%ah\n"
        "mov $0x00, %%bh\n"
        "mov %0, %%al\n"
        "int $0x10"
        : : "r"(c) : "ax", "bx"
    );
}

void print_string(const char *str) {
    while (*str) {
        print_char(*str);
        str++;
    }
}

void load_kernel() {
    print_string("Loading kernel...\r\n");
}

int main() {
    print_string("Stage 2 loaded!\r\n");
    load_kernel();
    asm volatile ("jmp 0x1000");
    return 0;
}