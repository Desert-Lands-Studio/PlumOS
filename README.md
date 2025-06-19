# PlumOS

PlumOS is an experimental operating system written mostly in Rust with some C/C++ and assembly bits. The goal is to build a small modular kernel that can eventually run on multiple architectures (x86_64, aarch64, riscv64 and more).

This repository currently contains early boot code and a very small Rust kernel. The project is in a very early stage and does not build out of the box yet. The code is provided as a starting point for experimentation.

## Building

A custom target file is used for bare‑metal compilation. For x86_64 the target description is located at `kernel/arch/x86_64/boot/x86_64-plumos.json`.

To build the kernel for x86_64 you will need a nightly toolchain with the
`rust-src` component installed. Enable `build-std` so that the `core`
library is built for the custom target.

```bash
rustup toolchain install nightly --component rust-src
cd kernel
cargo +nightly build -Z build-std=core --target arch/x86_64/boot/x86_64-plumos.json
```

Alternatively, running `cargo build` in the repository root will attempt the
same build.

## Architecture directories

Architecture specific boot code lives under `kernel/arch/<arch>/boot/`. An
initial AArch64 stub has been added as an example.

