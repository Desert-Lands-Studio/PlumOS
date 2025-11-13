ROOT_DIR ?= $(CURDIR)

ARCH ?= $(shell uname -m)
ifeq ($(ARCH),x86_64)
	TARGET_TRIPLE := x86_64-unknown-none
	QEMU_SYSTEM := qemu-system-x86_64
else ifeq ($(ARCH),aarch64)
	TARGET_TRIPLE := aarch64-unknown-none
	QEMU_SYSTEM := qemu-system-aarch64
else ifeq ($(ARCH),riscv64)
	TARGET_TRIPLE := riscv64-unknown-none
	QEMU_SYSTEM := qemu-system-riscv64
else
$(error Unsupported ARCH: $(ARCH))
endif

OUTPUT_DIR := $(ROOT_DIR)/build/output
BUILD_DIR  := $(ROOT_DIR)/build/$(ARCH)

BOOTLOADER_BIN := $(OUTPUT_DIR)/bootloader-$(ARCH).bin
KERNEL_PLAM    := $(OUTPUT_DIR)/kernel-$(ARCH).plam
MKPLAM_TOOL    := $(ROOT_DIR)/tools/mkplam

IMAGE     := $(OUTPUT_DIR)/plumos-$(ARCH).img
LIVE_ISO  := $(OUTPUT_DIR)/plumos-$(ARCH)-live.iso

$(shell mkdir -p $(OUTPUT_DIR))