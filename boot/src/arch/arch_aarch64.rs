// src/arch/arch_aarch64.rs
#[no_mangle]
pub extern "C" fn arch_boot_kernel(entry_point: u64) -> ! {
    unsafe {
        asm!(
            "br {0}",
            in(reg) entry_point,
            options(noreturn)
        );
    }
}
