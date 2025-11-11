# === Kernel Makefile ===
.PHONY: all clean

KERNEL_TARGET ?= aarch64-unknown-none
KERNEL_OUTPUT ?= ../build/output/kernel-$(KERNEL_TARGET).plam

all: $(KERNEL_OUTPUT)

$(KERNEL_OUTPUT): pmhk pmm | ../build/output
	@if [ -f ../tools/mkplam ]; then \
		../tools/mkplam ../target/$(KERNEL_TARGET)/release/kernel $@ --arch=$(shell echo $(KERNEL_TARGET) | cut -d'-' -f1); \
	else \
		cp ../target/$(KERNEL_TARGET)/release/kernel $@; \
		echo "⚠️ mkplam not found, using raw kernel"; \
	fi

pmhk:
	cargo build --manifest-path ./pmhk/Cargo.toml --target $(KERNEL_TARGET) --release

pmm:
	cargo build --manifest-path ./pmm/Cargo.toml --target $(KERNEL_TARGET) --release

../build/output:
	mkdir -p $@

clean:
	rm -f $(KERNEL_OUTPUT)
	cargo clean --manifest-path ./pmhk/Cargo.toml
	cargo clean --manifest-path ./pmm/Cargo.toml