BUILD_DIR = build
KERNEL_DIR = sys
BOOTLOADER_DIR = bootloader

CC = gcc
CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra

all: kernel bootloader

kernel:
    $(CC) $(CFLAGS) -c $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
    ld -T $(KERNEL_DIR)/link.ld -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o

bootloader:
    $(MAKE) -C $(BOOTLOADER_DIR)

clean:
    rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.elf
