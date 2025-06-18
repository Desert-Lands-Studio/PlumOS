#!/usr/bin/env bash
set -e

# Конфигурация
TARGET=x86_64-unknown-uefi
BOOT_EFI=target/$TARGET/release/plum_bootloader.efi
KERNEL=kernel.plam
ESP_SIZE=64M
ESP_IMG=esp.img
OVMF_PATH=${OVMF_PATH:-/usr/share/ovmf/OVMF.fd}

# Инструкции для Windows
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    echo "Running on Windows. Ensure you have Git Bash or WSL installed."
    echo "Install mtools: 'pacman -S mtools' (MSYS2) or 'sudo apt install mtools' (WSL)"
    echo "Install QEMU: Download from https://www.qemu.org/download/ and add to PATH"
    echo "OVMF: Download OVMF.fd and set OVMF_PATH environment variable"
fi

# Проверка зависимостей
command -v cargo >/dev/null 2>&1 || { echo "Error: cargo is not installed"; exit 1; }
command -v mcopy >/dev/null 2>&1 || { echo "Error: mtools is not installed"; exit 1; }
command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "Error: qemu-system-x86_64 is not installed"; exit 1; }

# Проверка kernel.plam
if [ ! -f "$KERNEL" ]; then
    echo "Error: $KERNEL not found! Please place it in the project root."
    exit 1
fi

# Проверка OVMF
if [ ! -f "$OVMF_PATH" ]; then
    echo "Error: OVMF firmware not found at $OVMF_PATH"
    exit 1
fi

# Сборка bootloader
echo "Building PLUM bootloader..."
cargo build --release --target $TARGET

# Проверка успешности сборки
if [ ! -f "$BOOT_EFI" ]; then
    echo "Error: Bootloader build failed!"
    exit 1
fi

# Создание FAT32-образа
echo "Creating ESP image..."
rm -f $ESP_IMG
truncate -s $ESP_SIZE $ESP_IMG
mkfs.fat -F32 -n PLUMBOOT $ESP_IMG

# Копирование файлов
echo "Copying files to ESP..."
mmd -i $ESP_IMG ::/EFI
mmd -i $ESP_IMG ::/EFI/BOOT
mcopy -i $ESP_IMG $BOOT_EFI ::/EFI/BOOT/BOOTX64.EFI
mcopy -i $ESP_IMG $KERNEL ::/kernel.plam

echo "ESP ready: $ESP_IMG"

# Тест в QEMU
echo "Running QEMU test..."
qemu-system-x86_64 -bios "$OVMF_PATH" \
    -drive if=none,format=raw,file=$ESP_IMG,id=esp \
    -device ide-hd,drive=esp -serial stdio -no-reboot