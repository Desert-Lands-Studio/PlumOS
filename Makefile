ROOT_DIR := $(CURDIR)

# Подключаем модули
include $(ROOT_DIR)/mk/config.mk
include $(ROOT_DIR)/mk/depends.mk

.PHONY: all clean rebuild help \
        image live run debug

all: $(IMAGE)

image: $(IMAGE)
live: $(LIVE_ISO)

run: $(IMAGE)
	$(MAKE) -f $(ROOT_DIR)/mk/qemu.mk run-qemu

debug: $(IMAGE)
	$(MAKE) -f $(ROOT_DIR)/mk/qemu.mk debug-qemu

$(BOOTLOADER_BIN):
	$(MAKE) -C bootloader build-$(ARCH)

$(KERNEL_PLAM): $(MKPLAM_TOOL)
	$(MAKE) -C kernel KERNEL_TARGET=$(TARGET_TRIPLE) KERNEL_OUTPUT=$@

$(MKPLAM_TOOL):
	$(MAKE) -C tools

include $(ROOT_DIR)/mk/disk.mk

clean:
	$(MAKE) -C bootloader clean
	$(MAKE) -C kernel clean
	$(MAKE) -C tools clean
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR)

rebuild: clean all

help:
	@echo "PlumOS Build System (Multi-Arch)"
	@echo ""
	@echo "Usage: make [target] ARCH=[x86_64|aarch64|riscv64]"
	@echo ""
	@echo "Targets:"
	@echo "  all             — build image (default)"
	@echo "  image           — build raw disk image"
	@echo "  live            — build UEFI Live ISO"
	@echo "  run             — run in QEMU"
	@echo "  debug           — run in QEMU with GDB stub (-s -S)"
	@echo "  clean           — clean all outputs"
	@echo "  rebuild         — full rebuild"
	@echo ""
	@echo "Examples:"
	@echo "  make ARCH=x86_64 all"
	@echo "  make ARCH=aarch64 run"
	@echo "  make ARCH=x86_64 debug"