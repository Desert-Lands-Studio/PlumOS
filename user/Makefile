# === User Apps Makefile ===
.PHONY: all clean

APPS = psh ppm-cli ppm-server

all: $(APPS)

psh:
	cargo build --manifest-path ../user/utils/psh/Cargo.toml --release
	cp ../target/release/psh ../build/output/sysroot/bin/

ppm-cli:
	cargo build --manifest-path ../user/utils/ppm/crates/cli/Cargo.toml --release
	cp ../target/release/ppm-cli ../build/output/sysroot/bin/ppm

ppm-server:
	cargo build --manifest-path ../user/servers/ppm-server/Cargo.toml --release
	cp ../target/release/ppm-server ../build/output/sysroot/bin/

../build/output/sysroot/bin:
	mkdir -p ../build/output/sysroot/bin

$(APPS): | ../build/output/sysroot/bin

clean:
	rm -rf ../build/output/sysroot
	cargo clean --manifest-path ../user/utils/psh/Cargo.toml
	cargo clean --manifest-path ../user/utils/ppm/crates/cli/Cargo.toml
	cargo clean --manifest-path ../user/servers/ppm-server/Cargo.toml