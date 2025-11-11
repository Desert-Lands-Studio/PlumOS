use std::env;

fn main() {
    let target = env::var("TARGET").unwrap();

    match target.as_str() {
        "aarch64-unknown-none" => compile_aarch64(),
        "riscv64-unknown-none" => compile_riscv64(),
        _ => {}
    }

    println!("cargo:rerun-if-changed=arch/");
}

fn compile_aarch64() {
    cc::Build::new()
        .compiler("clang")
        .file("arch/aarch64/boot/baremetal/start.S")
        .flag("-target")
        .flag("aarch64-unknown-none")
        .compile("aarch64-start");

    cc::Build::new()
        .compiler("clang")
        .file("arch/aarch64/mmio.c")
        .flag("-target")
        .flag("aarch64-unknown-none")
        .flag("-ffreestanding")
        .compile("aarch64-mmio");

    println!("cargo:rerun-if-changed=arch/aarch64/boot/baremetal/start.S");
    println!("cargo:rerun-if-changed=arch/aarch64/mmio.c");
}

fn compile_riscv64() {
    cc::Build::new()
        .file("arch/riscv64/clint.c")
        .target("riscv64-none-elf")
        .flag("-ffreestanding")
        .compile("riscv64-clint");
        
    println!("cargo:rerun-if-changed=arch/riscv64/clint.c");
}