use crate::arch::{Arch, KernelInfo};
pub struct AArch64;

impl Arch for AArch64 {
    fn get_kernel_info() -> KernelInfo {
        KernelInfo {
            entry_point: 0x40080000,
            memory_map: &[],
            cmdline: "console=ttyAMA0",
            #[cfg(target_arch = "aarch64")]
            device_tree_ptr: Some(0x40000000),
            #[cfg(not(target_arch = "aarch64"))]
            device_tree_ptr: None,
        }
    }

    fn jump_to_kernel(info: KernelInfo) -> ! {
        unsafe {
            #[cfg(target_arch = "aarch64")]
            {
                if let Some(dtb) = info.device_tree_ptr {
                    core::arch::asm!(
                        "mov x0, {dtb}",
                        "br {entry}",
                        dtb = in(reg) dtb,
                        entry = in(reg) info.entry_point,
                        options(noreturn)
                    );
                } else {
                    core::arch::asm!(
                        "br {entry}",
                        entry = in(reg) info.entry_point,
                        options(noreturn)
                    );
                }
            }
            #[cfg(not(target_arch = "aarch64"))]
            {
                core::arch::asm!(
                    "jmp {entry}",
                    entry = in(reg) info.entry_point,
                    options(noreturn)
                );
            }
        }
    }
}