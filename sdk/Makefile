# === SDK Makefile ===
.PHONY: all clean

SDK_CRATES = plum-hal plum-formats plum-abi ppm-core

all: $(SDK_CRATES)

$(SDK_CRATES):
	cargo build --manifest-path ./lib/$@/Cargo.toml --release

clean:
	@for crate in $(SDK_CRATES); do \
		cargo clean --manifest-path ./lib/$$crate/Cargo.toml; \
	done