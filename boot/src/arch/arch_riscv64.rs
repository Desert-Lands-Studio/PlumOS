// src/arch/arch_riscv64.rs
#[no_mangle]
pub extern "C" fn arch_boot_kernel(entry_point: u64) -> ! {
    unsafe {
        asm!(
            "jr {0}",
            in(reg) entry_point,
            options(noreturn)
        );
    }
}
