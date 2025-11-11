#[cfg(target_arch = "aarch64")]
pub mod aarch64;
#[cfg(target_arch = "x86_64")]
pub mod x86_64;
#[cfg(target_arch = "riscv64")]
pub mod riscv64;

pub struct KernelInfo {
    pub entry_point: usize,
    pub memory_map: &'static [u8],
    pub cmdline: &'static str,
    pub device_tree_ptr: Option<usize>,
}

pub trait Arch {
    fn get_kernel_info() -> KernelInfo;
    fn jump_to_kernel(info: KernelInfo) -> !;
}