use super::Arch;
use super::KernelInfo;

pub struct RiscV64;

impl Arch for RiscV64 {
    fn init_early() {
        
        
        unsafe {
            clint_init();
        }
    }
    
    fn init_memory() {
        
    }
    
    fn get_kernel_info() -> KernelInfo {
        KernelInfo {
            entry_point: 0x80000000, 
            memory_map: &[],
            cmdline: "console=ttyS0",
        }
    }
    
    fn jump_to_kernel() -> ! {
        let info = Self::get_kernel_info();
        unsafe {
            core::arch::asm!(
                "jr {}",
                in(reg) info.entry_point,
                options(noreturn)
            );
        }
    }
}


extern "C" {
    fn clint_init();
}