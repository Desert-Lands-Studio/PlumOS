QEMU_COMMON := -serial mon:stdio -m 2G -smp 4

ifeq ($(ARCH),x86_64)
	QEMU_FLAGS := $(QEMU_COMMON) -M q35 -cpu qemu64
	QEMU_DRIVE := -drive format=raw,file=$(IMAGE)
	LIVE_FLAGS := -drive format=raw,file=$(LIVE_ISO),if=pflash,readonly=on
endif

ifeq ($(ARCH),aarch64)
	QEMU_FLAGS := $(QEMU_COMMON) -M virt -cpu cortex-a57
	QEMU_DRIVE := -drive if=none,file=$(IMAGE),id=hd0 -device virtio-blk-device,drive=hd0
	LIVE_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(LIVE_ISO)
endif

ifeq ($(ARCH),riscv64)
	QEMU_FLAGS := $(QEMU_COMMON) -M virt -cpu rv64
	QEMU_DRIVE := -drive if=none,file=$(IMAGE),id=hd0 -device virtio-blk-device,drive=hd0
	LIVE_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(LIVE_ISO)
endif

run-qemu: $(IMAGE)
	$(QEMU_SYSTEM) $(QEMU_FLAGS) $(QEMU_DRIVE)

debug-qemu: $(IMAGE)
	$(QEMU_SYSTEM) $(QEMU_FLAGS) $(QEMU_DRIVE) -s -S

run-live: $(LIVE_ISO)
	$(QEMU_SYSTEM) $(QEMU_FLAGS) $(LIVE_FLAGS)

.PHONY: run-qemu debug-qemu run-live