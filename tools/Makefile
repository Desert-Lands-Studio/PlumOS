# === Tools Makefile ===
.PHONY: all clean

TOOLS = mkplam plum-config

all: $(TOOLS)

mkplam:
	cargo build --manifest-path ./mkplam/Cargo.toml --release
	cp ../target/release/mkplam .

plum-config:
	cargo build --manifest-path ./plum-config/Cargo.toml --release
	cp ../target/release/plum-config .

clean:
	rm -f $(TOOLS)
	cargo clean --manifest-path ./mkplam/Cargo.toml
	cargo clean --manifest-path ./plum-config/Cargo.toml