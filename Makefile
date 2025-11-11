# === Главный Makefile PlumOS ===
.PHONY: all clean help \
        build-bootloader build-kernel build-sdk build-tools build-user \
        image-aarch64 image-x86_64-bios image-x86_64-uefi \
        run-qemu-aarch64 run-qemu-x86_64-bios run-qemu-x86_64-uefi

# === Подцели ===
build-bootloader:
	$(MAKE) -C bootloader

build-kernel:
	$(MAKE) -C kernel

build-sdk:
	$(MAKE) -C sdk

build-tools:
	$(MAKE) -C tools

build-user:
	$(MAKE) -C user

# === Сборка всего ===
all: build-sdk build-bootloader build-kernel build-tools build-user

# === Образы ===
image-aarch64:
	$(MAKE) -C bootloader image-aarch64

image-x86_64-bios:
	$(MAKE) -C bootloader image-x86_64-bios

image-x86_64-uefi:
	$(MAKE) -C bootloader image-x86_64-uefi

# === Запуск ===
run-qemu-aarch64:
	$(MAKE) -C bootloader run-qemu-aarch64

run-qemu-x86_64-bios:
	$(MAKE) -C bootloader run-qemu-x86_64-bios

run-qemu-x86_64-uefi:
	$(MAKE) -C bootloader run-qemu-x86_64-uefi

# === Очистка ===
clean:
	$(MAKE) -C bootloader clean
	$(MAKE) -C kernel clean
	$(MAKE) -C sdk clean
	$(MAKE) -C tools clean
	$(MAKE) -C user clean
	rm -rf target build/output

help:
	@echo "PlumOS Modular Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all                      – Build all components"
	@echo "  build-bootloader         – Build all bootloaders"
	@echo "  build-kernel             – Build kernel (pmhk + pmm)"
	@echo "  build-sdk                – Build PlumOS SDK"
	@echo "  build-tools              – Build tools (mkplam, etc.)"
	@echo "  build-user               – Build user apps (psh, ppm)"
	@echo ""
	@echo "  image-aarch64            – Create AArch64 disk image"
	@echo "  image-x86_64-bios        – Create x86_64 BIOS image"
	@echo "  image-x86_64-uefi        – Create x86_64 UEFI image"
	@echo ""
	@echo "  run-qemu-aarch64         – Run AArch64 in QEMU"
	@echo "  run-qemu-x86_64-bios     – Run x86_64 BIOS in QEMU"
	@echo "  run-qemu-x86_64-uefi     – Run x86_64 UEFI in QEMU"
	@echo ""
	@echo "  clean                    – Clean all build artifacts"