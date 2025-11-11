#include <stdint.h>

#define UART_BASE 0x09000000UL

static inline void mmio_write(uint32_t reg, uint32_t data) {
    *(volatile uint32_t *)((uintptr_t)UART_BASE + reg) = data;
}

static inline uint32_t mmio_read(uint32_t reg) {
    return *(volatile uint32_t *)((uintptr_t)UART_BASE + reg);
}

void uart_init() {
    mmio_write(0x30, 0x00);
    mmio_write(0x24, 0x02);
    mmio_write(0x28, 0x00); // FBRD = 0
    mmio_write(0x2C, (1 << 4) | (3 << 5)); // LCR_H
    mmio_write(0x30, (1 << 0) | (1 << 8) | (1 << 9)); // CR
}

void uart_putc(char c) {
    while (mmio_read(0x18) & 0x20);
    mmio_write(0x00, c);
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

void memzero(void *ptr, unsigned long size) {
    uint8_t *p = (uint8_t*)ptr;
    for (unsigned long i = 0; i < size; i++) {
        p[i] = 0;
    }
}