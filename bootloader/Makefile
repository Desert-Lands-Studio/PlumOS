# === Bootloader Makefile ===
.PHONY: all clean \
        build-aarch64 build-x86_64-bios build-x86_64-uefi \
        image-aarch64 image-x86_64-bios image-x86_64-uefi \
        run-qemu-aarch64 run-qemu-x86_64-bios run-qemu-x86_64-uefi

OUTPUT_DIR = ../build/output
MKPLAM = ../tools/mkplam

# Аутпуты
BOOT_AARCH64 = $(OUTPUT_DIR)/bootloader-aarch64.bin
KERNEL_AARCH64 = $(OUTPUT_DIR)/kernel-aarch64.plam
IMG_AARCH64 = $(OUTPUT_DIR)/plumos-aarch64.img

BOOT_X86_BIOS = $(OUTPUT_DIR)/bootloader-x86_64-bios.bin
KERNEL_X86 = $(OUTPUT_DIR)/kernel-x86_64.plam
IMG_X86_BIOS = $(OUTPUT_DIR)/plumos-x86_64-bios.img

BOOT_X86_UEFI = $(OUTPUT_DIR)/bootloader-x86_64-uefi.efi
KERNEL_X86_UEFI = $(OUTPUT_DIR)/kernel-x86_64-uefi.plam
IMG_X86_UEFI = $(OUTPUT_DIR)/plumos-x86_64-uefi.img

# QEMU
QEMU_AARCH64 = qemu-system-aarch64 -M virt -cpu cortex-a53 -smp 4 -m 2G -nographic -serial mon:stdio
QEMU_X86_BIOS = qemu-system-x86_64 -M pc -cpu qemu64 -smp 4 -m 2G -nographic -serial mon:stdio
QEMU_X86_UEFI = qemu-system-x86_64 -M q35 -cpu qemu64 -smp 4 -m 2G -nographic -serial mon:stdio \
                -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
                -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_VARS.fd

$(OUTPUT_DIR):
	mkdir -p $@

# === Сборка ===
build-aarch64: $(BOOT_AARCH64)
build-x86_64-bios: $(BOOT_X86_BIOS)
build-x86_64-uefi: $(BOOT_X86_UEFI)

$(BOOT_AARCH64): | $(OUTPUT_DIR)
	cargo build --manifest-path ./Cargo.toml --target aarch64-unknown-none --release
	rust-objcopy -O binary ../target/aarch64-unknown-none/release/bootloader $@

$(BOOT_X86_BIOS): | $(OUTPUT_DIR)
	cargo build --manifest-path ./Cargo.toml --target x86_64-unknown-none --release
	rust-objcopy -O binary ../target/x86_64-unknown-none/release/bootloader $@

$(BOOT_X86_UEFI): | $(OUTPUT_DIR)
	cargo build --manifest-path ./Cargo.toml --target x86_64-unknown-uefi --release
	cp ../target/x86_64-unknown-uefi/release/bootloader.efi $@

# === Образы ===
image-aarch64: $(BOOT_AARCH64) $(KERNEL_AARCH64) | $(OUTPUT_DIR)
	dd if=/dev/zero of=$(IMG_AARCH64) bs=1M count=64
	dd if=$(BOOT_AARCH64) of=$(IMG_AARCH64) conv=notrunc
	dd if=$(KERNEL_AARCH64) of=$(IMG_AARCH64) seek=2048 conv=notrunc

image-x86_64-bios: $(BOOT_X86_BIOS) $(KERNEL_X86) | $(OUTPUT_DIR)
	dd if=/dev/zero of=$(IMG_X86_BIOS) bs=1M count=64
	dd if=$(BOOT_X86_BIOS) of=$(IMG_X86_BIOS) conv=notrunc
	dd if=$(KERNEL_X86) of=$(IMG_X86_BIOS) seek=2048 conv=notrunc

image-x86_64-uefi: $(BOOT_X86_UEFI) $(KERNEL_X86_UEFI) | $(OUTPUT_DIR)
	dd if=/dev/zero of=$(IMG_X86_UEFI) bs=1M count=64
	mkdir -p efi/boot
	cp $(BOOT_X86_UEFI) efi/boot/bootx64.efi
	cp $(KERNEL_X86_UEFI) efi/kernel.plam
	mformat -i $(IMG_X86_UEFI) -C -F -v "PLUMOS_UEFI" ::
	mcopy -i $(IMG_X86_UEFI) efi/boot/bootx64.efi ::
	mcopy -i $(IMG_X86_UEFI) efi/kernel.plam ::
	rm -rf efi

# Зависимости ядра
$(KERNEL_AARCH64):
	$(MAKE) -C ../kernel KERNEL_TARGET=aarch64-unknown-none KERNEL_OUTPUT=$(KERNEL_AARCH64)

$(KERNEL_X86) $(KERNEL_X86_UEFI):
	$(MAKE) -C ../kernel KERNEL_TARGET=x86_64-unknown-none KERNEL_OUTPUT=$(KERNEL_X86)

# === Запуск ===
run-qemu-aarch64: image-aarch64
	$(QEMU_AARCH64) -drive if=none,file=$(IMG_AARCH64),id=hd0,format=raw -device virtio-blk-device,drive=hd0

run-qemu-x86_64-bios: image-x86_64-bios
	$(QEMU_X86_BIOS) -drive format=raw,file=$(IMG_X86_BIOS)

run-qemu-x86_64-uefi: image-x86_64-uefi
	$(QEMU_X86_UEFI) -drive format=raw,file=$(IMG_X86_UEFI)

# === Очистка ===
clean:
	rm -f $(BOOT_AARCH64) $(BOOT_X86_BIOS) $(BOOT_X86_UEFI)
	rm -f $(IMG_AARCH64) $(IMG_X86_BIOS) $(IMG_X86_UEFI)s