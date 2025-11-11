.PHONY: all config build-aarch64 build-x86_64 build-x86_64-bios build-x86_64-uefi \
        build-bootloader-aarch64 build-bootloader-x86_64-bios build-bootloader-x86_64-uefi \
        build-kernel-aarch64 build-kernel-x86_64 \
        image-aarch64 image-x86_64-bios image-x86_64-uefi \
        run-qemu-aarch64 run-qemu-x86_64-bios run-qemu-x86_64-uefi run-dev \
        build-ppm build-psh build-ppm-server install-ppm clean \
        debug-bootloader list-targets help

# === Пути проекта ===
PPM_DIR = user/utils/ppm
PPM_CLI_DIR = user/utils/ppm/crates/cli
PPM_KEYGEN_DIR = user/utils/ppm/crates/keygen
PPM_TUI_DIR = user/utils/ppm/crates/tui

PSH_DIR = user/utils/psh
PPM_SERVER_DIR = user/servers/ppm-server

# === Выходные артефакты ===
OUTPUT_DIR = build/output
SYSROOT = $(OUTPUT_DIR)/sysroot

# AArch64
BOOTLOADER_AARCH64_BIN = $(OUTPUT_DIR)/bootloader-aarch64.bin
KERNEL_AARCH64_RAW = $(OUTPUT_DIR)/kernel-aarch64.raw
KERNEL_AARCH64_PLAM = $(OUTPUT_DIR)/kernel-aarch64.plam
IMG_AARCH64 = $(OUTPUT_DIR)/plumos-aarch64.img

# x86_64 BIOS
BOOTLOADER_X86_64_BIOS_BIN = $(OUTPUT_DIR)/bootloader-x86_64-bios.bin
BOOTLOADER_X86_64_BIOS_ELF = $(OUTPUT_DIR)/bootloader-x86_64-bios.elf
KERNEL_X86_64_RAW = $(OUTPUT_DIR)/kernel-x86_64.raw
KERNEL_X86_64_PLAM = $(OUTPUT_DIR)/kernel-x86_64.plam
IMG_X86_64_BIOS = $(OUTPUT_DIR)/plumos-x86_64-bios.img

# x86_64 UEFI
BOOTLOADER_X86_64_UEFI_EFI = $(OUTPUT_DIR)/bootloader-x86_64-uefi.efi
KERNEL_X86_64_UEFI_PLAM = $(OUTPUT_DIR)/kernel-x86_64-uefi.plam
IMG_X86_64_UEFI = $(OUTPUT_DIR)/plumos-x86_64-uefi.img

# === QEMU ===
QEMU_AARCH64_FLAGS = -M virt -cpu cortex-a53 -smp 4 -m 2G \
                     -drive if=none,file=$(IMG_AARCH64),id=hd0,format=raw \
                     -device virtio-blk-device,drive=hd0 \
                     -nographic -serial mon:stdio -d guest_errors -D qemu.log

QEMU_X86_64_BIOS_FLAGS = -M pc -cpu qemu64 -smp 4 -m 2G \
                         -drive format=raw,file=$(IMG_X86_64_BIOS) \
                         -nographic -serial mon:stdio -d guest_errors -D qemu.log

QEMU_X86_64_UEFI_FLAGS = -M q35 -cpu qemu64 -smp 4 -m 2G \
                         -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
                         -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_VARS.fd \
                         -drive format=raw,file=$(IMG_X86_64_UEFI) \
                         -nographic -serial mon:stdio -d guest_errors -D qemu.log

# === Цели по умолчанию ===
all: build-aarch64 build-ppm build-psh

# === Директории ===
$(OUTPUT_DIR):
	mkdir -p $@

# === Сборка загрузчиков ===

# AArch64
build-bootloader-aarch64: $(BOOTLOADER_AARCH64_BIN)

$(BOOTLOADER_AARCH64_BIN): | $(OUTPUT_DIR)
	@echo "🔨 Building AArch64 Bootloader..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path bootloader/Cargo.toml --target aarch64-unknown-none --release
	
	@echo "📦 Extracting binary..."
	ELF_FILE="target/aarch64-unknown-none/release/bootloader"; \
	if [ ! -f "$$ELF_FILE" ]; then \
		echo "❌ Bootloader ELF not found at $$ELF_FILE!"; \
		echo "Available files in target/aarch64-unknown-none/release/:"; \
		ls -la target/aarch64-unknown-none/release/ 2>/dev/null || echo "Directory not found"; \
		exit 1; \
	fi; \
	
	@echo "📋 Checking ELF sections..."
	rust-objdump -h "$$ELF_FILE" | head -20 || true
	
	rust-objcopy -O binary "$$ELF_FILE" "$@"
	
	@echo "✅ Bootloader built: $@"
	@echo "📏 Bootloader size: $$(wc -c < "$@") bytes"
	@if [ -s "$@" ]; then \
		echo "🔍 First 64 bytes:"; \
		hexdump -C "$@" | head -5; \
	else \
		echo "❌ Binary is empty!"; \
		exit 1; \
	fi

# x86_64 BIOS
build-bootloader-x86_64-bios: $(BOOTLOADER_X86_64_BIOS_BIN)

$(BOOTLOADER_X86_64_BIOS_BIN): | $(OUTPUT_DIR)
	@echo "🔨 Building x86_64 BIOS Bootloader..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path bootloader/Cargo.toml --target x86_64-unknown-none --release
	
	@echo "📦 Extracting binary..."
	ELF_FILE="target/x86_64-unknown-none/release/bootloader"; \
	if [ ! -f "$$ELF_FILE" ]; then \
		echo "❌ Bootloader ELF not found at $$ELF_FILE!"; \
		exit 1; \
	fi; \
	
	cp "$$ELF_FILE" $(BOOTLOADER_X86_64_BIOS_ELF)
	rust-objcopy -O binary "$$ELF_FILE" "$@"
	
	@echo "✅ x86_64 BIOS Bootloader built: $@"
	@echo "📏 Bootloader size: $$(wc -c < "$@") bytes"

# x86_64 UEFI
build-bootloader-x86_64-uefi: $(BOOTLOADER_X86_64_UEFI_EFI)

$(BOOTLOADER_X86_64_UEFI_EFI): | $(OUTPUT_DIR)
	@echo "🔨 Building x86_64 UEFI Bootloader..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path bootloader/Cargo.toml --target x86_64-unknown-uefi --release
	
	@echo "📦 Copying EFI file..."
	EFI_FILE="target/x86_64-unknown-uefi/release/bootloader.efi"; \
	if [ ! -f "$$EFI_FILE" ]; then \
		echo "❌ Bootloader EFI not found at $$EFI_FILE!"; \
		exit 1; \
	fi; \
	
	cp "$$EFI_FILE" "$@"
	
	@echo "✅ x86_64 UEFI Bootloader built: $@"
	@echo "📏 Bootloader size: $$(wc -c < "$@") bytes"

# === Сборка ядер ===

# AArch64 kernel
build-kernel-aarch64: $(KERNEL_AARCH64_PLAM)

$(KERNEL_AARCH64_PLAM): $(KERNEL_AARCH64_RAW) | $(OUTPUT_DIR)
	@echo "📦 Packing AArch64 kernel into .plam format..."
	if [ -f tools/mkplam ]; then \
		tools/mkplam $< $@ --arch=aarch64; \
	else \
		cp $< $@; \
		echo "⚠️  mkplam tool not found, using raw kernel as .plam"; \
	fi

$(KERNEL_AARCH64_RAW): | $(OUTPUT_DIR)
	@echo "🔨 Building AArch64 Kernel (raw)..."
	CARGO_TARGET_DIR=$(abspath target) RUSTFLAGS="-C link-arg=-T$(abspath kernel/pmhk/linkers/aarch64.ld)" \
		cargo build --manifest-path kernel/pmhk/Cargo.toml --target aarch64-unknown-none --release
	cp target/aarch64-unknown-none/release/kernel $@
	@echo "✅ AArch64 Kernel raw: $@"

# x86_64 kernel
build-kernel-x86_64: $(KERNEL_X86_64_PLAM)

$(KERNEL_X86_64_PLAM): $(KERNEL_X86_64_RAW) | $(OUTPUT_DIR)
	@echo "📦 Packing x86_64 kernel into .plam format..."
	if [ -f tools/mkplam ]; then \
		tools/mkplam $< $@ --arch=x86_64; \
	else \
		cp $< $@; \
		echo "⚠️  mkplam tool not found, using raw kernel as .plam"; \
	fi

$(KERNEL_X86_64_RAW): | $(OUTPUT_DIR)
	@echo "🔨 Building x86_64 Kernel (raw)..."
	CARGO_TARGET_DIR=$(abspath target) RUSTFLAGS="-C link-arg=-T$(abspath kernel/pmhk/linkers/x86_64.ld)" \
		cargo build --manifest-path kernel/pmhk/Cargo.toml --target x86_64-unknown-none --release
	cp target/x86_64-unknown-none/release/kernel $@
	@echo "✅ x86_64 Kernel raw: $@"

# x86_64 UEFI kernel
build-kernel-x86_64-uefi: $(KERNEL_X86_64_UEFI_PLAM)

$(KERNEL_X86_64_UEFI_PLAM): | $(OUTPUT_DIR)
	@echo "🔨 Building x86_64 UEFI Kernel..."
	# Для UEFI может потребоваться специальная сборка
	$(MAKE) build-kernel-x86_64
	cp $(KERNEL_X86_64_PLAM) $@
	@echo "✅ x86_64 UEFI Kernel: $@"

# === Основные цели сборки ===

# AArch64
build-aarch64: build-bootloader-aarch64 build-kernel-aarch64
	@echo "✅ AArch64 build complete"

# x86_64 BIOS
build-x86_64-bios: build-bootloader-x86_64-bios build-kernel-x86_64
	@echo "✅ x86_64 BIOS build complete"

# x86_64 UEFI
build-x86_64-uefi: build-bootloader-x86_64-uefi build-kernel-x86_64-uefi
	@echo "✅ x86_64 UEFI build complete"

# Все x86_64 цели
build-x86_64: build-x86_64-bios build-x86_64-uefi
	@echo "✅ All x86_64 builds complete"

# Все архитектуры
build-all: build-aarch64 build-x86_64
	@echo "✅ All architectures built"

# === Образы дисков ===

# AArch64 image
image-aarch64: build-aarch64 install-ppm
	@echo "📀 Creating AArch64 disk image..."
	dd if=/dev/zero of=$(IMG_AARCH64) bs=1M count=64 status=progress
	dd if=$(BOOTLOADER_AARCH64_BIN) of=$(IMG_AARCH64) conv=notrunc status=none
	dd if=$(KERNEL_AARCH64_PLAM) of=$(IMG_AARCH64) seek=2048 conv=notrunc status=none
	@echo "✅ AArch64 image created: $(IMG_AARCH64)"

# x86_64 BIOS image
image-x86_64-bios: build-x86_64-bios install-ppm
	@echo "📀 Creating x86_64 BIOS disk image..."
	dd if=/dev/zero of=$(IMG_X86_64_BIOS) bs=1M count=64 status=progress
	dd if=$(BOOTLOADER_X86_64_BIOS_BIN) of=$(IMG_X86_64_BIOS) conv=notrunc status=none
	dd if=$(KERNEL_X86_64_PLAM) of=$(IMG_X86_64_BIOS) seek=2048 conv=notrunc status=none
	@echo "✅ x86_64 BIOS image created: $(IMG_X86_64_BIOS)"

# x86_64 UEFI image
image-x86_64-uefi: build-x86_64-uefi install-ppm
	@echo "📀 Creating x86_64 UEFI disk image..."
	dd if=/dev/zero of=$(IMG_X86_64_UEFI) bs=1M count=64 status=progress
	# Для UEFI нужно создать FAT файловую систему и поместить туда EFI файлы
	mkdir -p $(OUTPUT_DIR)/efi/boot
	cp $(BOOTLOADER_X86_64_UEFI_EFI) $(OUTPUT_DIR)/efi/boot/bootx64.efi
	cp $(KERNEL_X86_64_UEFI_PLAM) $(OUTPUT_DIR)/efi/kernel.plam
	mformat -i $(IMG_X86_64_UEFI) -C -F -v "PLUMOS_UEFI" ::
	mcopy -i $(IMG_X86_64_UEFI) $(OUTPUT_DIR)/efi/boot/bootx64.efi ::
	mcopy -i $(IMG_X86_64_UEFI) $(OUTPUT_DIR)/efi/kernel.plam ::
	rm -rf $(OUTPUT_DIR)/efi
	@echo "✅ x86_64 UEFI image created: $(IMG_X86_64_UEFI)"

# Все образы
image-all: image-aarch64 image-x86_64-bios image-x86_64-uefi
	@echo "✅ All disk images created"

# === Запуск в QEMU ===

run-qemu-aarch64: image-aarch64
	@echo "🚀 Starting QEMU AArch64..."
	qemu-system-aarch64 $(QEMU_AARCH64_FLAGS)

run-qemu-x86_64-bios: image-x86_64-bios
	@echo "🚀 Starting QEMU x86_64 BIOS..."
	qemu-system-x86_64 $(QEMU_X86_64_BIOS_FLAGS)

run-qemu-x86_64-uefi: image-x86_64-uefi
	@echo "🚀 Starting QEMU x86_64 UEFI..."
	qemu-system-x86_64 $(QEMU_X86_64_UEFI_FLAGS)

run-dev: build-aarch64
	@echo "⚡ Fast development boot (direct kernel)..."
	qemu-system-aarch64 $(QEMU_AARCH64_FLAGS) -kernel $(KERNEL_AARCH64_PLAM)

# === PPM и psh ===
build-ppm:
	@echo "🔨 Building PPM CLI..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path $(PPM_CLI_DIR)/Cargo.toml --release
	@echo "🔨 Building PPM Keygen..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path $(PPM_KEYGEN_DIR)/Cargo.toml --release
	@echo "🔨 Building PPM TUI..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path $(PPM_TUI_DIR)/Cargo.toml --release
	@echo "✅ PPM components built successfully"

build-psh:
	@echo "🐚 Building Plum Shell..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path $(PSH_DIR)/Cargo.toml --release
	@echo "✅ Plum Shell built successfully"

build-ppm-server:
	@echo "🌐 Building PPM Server..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path $(PPM_SERVER_DIR)/Cargo.toml --release
	@echo "✅ PPM Server built successfully"

install-ppm: build-ppm build-psh build-ppm-server | $(OUTPUT_DIR)
	@echo "📦 Installing PPM components..."
	mkdir -p $(SYSROOT)/bin
	mkdir -p $(SYSROOT)/etc/ppm
	mkdir -p $(SYSROOT)/var/lib/ppm/{stable,testing,unstable,dev}
	mkdir -p $(SYSROOT)/var/cache/ppm
	mkdir -p $(SYSROOT)/srv/ppm/{staging/pending,staging/approved,staging/rejected,web}

	cp target/release/ppm-cli $(SYSROOT)/bin/ppm
	cp target/release/psh $(SYSROOT)/bin/psh
	cp target/release/psh $(SYSROOT)/bin/sh
	cp target/release/ppm-server $(SYSROOT)/bin/ppm-server

	cp $(PPM_DIR)/assets/default-config.toml $(SYSROOT)/etc/ppm/config.toml

	mkdir -p $(SYSROOT)/srv/ppm/web/{admin,static}
	@echo "<h1>PPM Web Interface</h1>" > $(SYSROOT)/srv/ppm/web/index.html
	@echo "✅ PPM components installed to $(SYSROOT)"

# === Отладка ===
debug-bootloader:
	@echo "🐛 Debugging bootloader build..."
	CARGO_TARGET_DIR=$(abspath target) cargo build --manifest-path bootloader/Cargo.toml --target aarch64-unknown-none --release --verbose
	
	@echo "📋 ELF analysis:"
	ELF_FILE="target/aarch64-unknown-none/release/bootloader"; \
	if [ -f "$$ELF_FILE" ]; then \
		rust-objdump -h "$$ELF_FILE"; \
		echo "📋 Symbols:"; \
		rust-nm -C "$$ELF_FILE" | head -20; \
		echo "📋 Disassembly:"; \
		rust-objdump -d "$$ELF_FILE" | head -50; \
	else \
		echo "❌ Bootloader ELF not found!"; \
	fi

# === Прочее ===
clean:
	@echo "🧹 Cleaning..."
	rm -rf target
	rm -rf $(OUTPUT_DIR)
	@echo "✅ Clean complete"

list-targets:
	@echo "🎯 Available targets:"
	@echo "  Architecture-specific:"
	@echo "    build-aarch64          - Build kernel and bootloader for AArch64"
	@echo "    build-x86_64-bios      - Build for x86_64 BIOS"
	@echo "    build-x86_64-uefi      - Build for x86_64 UEFI"
	@echo "    build-x86_64           - Build all x86_64 variants"
	@echo "    build-all              - Build all architectures"
	@echo ""
	@echo "  Images:"
	@echo "    image-aarch64          - Create AArch64 bootable image"
	@echo "    image-x86_64-bios      - Create x86_64 BIOS image"
	@echo "    image-x86_64-uefi      - Create x86_64 UEFI image"
	@echo "    image-all              - Create all images"
	@echo ""
	@echo "  Running:"
	@echo "    run-qemu-aarch64       - Run AArch64 in QEMU"
	@echo "    run-qemu-x86_64-bios   - Run x86_64 BIOS in QEMU"
	@echo "    run-qemu-x86_64-uefi   - Run x86_64 UEFI in QEMU"
	@echo ""
	@echo "  Applications:"
	@echo "    build-ppm              - Build package manager"
	@echo "    build-psh              - Build shell"
	@echo "    install-ppm            - Install PPM to sysroot"
	@echo ""
	@echo "  Maintenance:"
	@echo "    clean                  - Clean build artifacts"
	@echo "    debug-bootloader       - Debug bootloader build"

help:
	@echo "PlumOS Build System"
	@echo ""
	@echo "Usage:"
	@echo "  make [target]"
	@echo ""
	@echo "Common targets:"
	@echo "  all                    - Build everything (AArch64 + apps)"
	@echo "  build-all              - Build all architectures"
	@echo "  image-all              - Create all disk images"
	@echo "  run-qemu-aarch64       - Run AArch64 in QEMU"
	@echo "  run-qemu-x86_64-bios   - Run x86_64 BIOS in QEMU"
	@echo "  build-ppm              - Build package manager only"
	@echo "  clean                  - Clean build artifacts"
	@echo ""
	@echo "For more targets: make list-targets"