ifeq ($(shell command -v rustc 2>/dev/null),)
$(error rustc not found — install via https://rustup.rs)
endif
ifeq ($(shell command -v cargo 2>/dev/null),)
$(error cargo not found)
endif
ifeq ($(ARCH),x86_64)
	ifeq ($(shell command -v nasm 2>/dev/null),)
	$(error nasm required for x86_64 bootloader)
	endif
endif
ifeq ($(shell command -v mcopy 2>/dev/null),)
$(error mtools required for UEFI ISO — install mtools)
endif
ifeq ($(shell command -v xorriso 2>/dev/null),)
$(error xorriso required for ISO — install xorriso)
endif