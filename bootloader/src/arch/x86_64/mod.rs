use super::Arch;
use super::KernelInfo;

pub struct X86_64;

impl Arch for X86_64 {
    fn init_early() {
        
        
        unsafe {
            gdt_load();
            idt_load();
        }
    }
    
    fn init_memory() {
        
    }
    
    fn get_kernel_info() -> KernelInfo {
        KernelInfo {
            entry_point: 0x100000, 
            memory_map: &[],
            cmdline: "console=ttyS0",
        }
    }
    
    fn jump_to_kernel() -> ! {
        let info = Self::get_kernel_info();
        unsafe {
            core::arch::asm!(
                "jmp {}",
                in(reg) info.entry_point,
                options(noreturn)
            );
        }
    }
}


extern "C" {
    fn gdt_load();
    fn idt_load();
}