#[cfg(target_arch = "aarch64")]
pub mod aarch64;
#[cfg(target_arch = "x86_64")]
pub mod x86_64;
#[cfg(target_arch = "riscv64")]
pub mod riscv64;

#[cfg(target_arch = "aarch64")]
pub unsafe fn init_early(dtb_ptr: usize) {
    aarch64::init_platform(dtb_ptr);
}

#[cfg(target_arch = "riscv64")]
pub unsafe fn init_early(dtb_ptr: usize) {
    // TODO: передать DTB или CLINT info
    riscv64::init_early();
    // Пока без DTB — использовать fallback
    let fallback = [(0x8000_0000, 0x800_0000)];
    pmm::init_buddy(&fallback, crate::config::config::PAGE_SIZE);
}

#[cfg(target_arch = "x86_64")]
pub unsafe fn init_early(_dtb_ptr: usize) {
    x86_64::init_early();
    // x86_64: пока без multiboot — fallback
    let fallback = [(0x100000, 0x800_0000)];
    pmm::init_buddy(&fallback, crate::config::config::PAGE_SIZE);
}