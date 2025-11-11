#![no_std]
#![no_main]
#![feature(alloc_error_handler)]
use core::panic::PanicInfo;
use plum_hal::uart::DefaultUart;
use spin::Mutex;
extern crate alloc;
use alloc::vec::Vec;
use crate::config::config::{ ARCH, PAGE_SIZE };
use crate::devicetree::DeviceTree;
use crate::kernel_alloc::init_heap;
use pmm::{ init_buddy, alloc_page };

mod arch;
mod config;
mod devicetree;
mod ipc;
mod init;
mod kernel_alloc;
mod types;
mod vfs;
mod mem;

static UART: Mutex<DefaultUart> = Mutex::new(DefaultUart::new(0x0900_0000));

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

#[no_mangle]
pub extern "C" fn _start(dtb_ptr: usize) -> ! {
    unsafe {
        let uart = 0x09000000 as *mut u32;
        let s = b"Kernel entry reached!\n";
        for &b in s {
            while (uart.add(6).read_volatile() & 0x20) == 0 {}
            uart.write_volatile(b as u32);
        }
    }
    
    let uart = UART.lock();
    uart.init();
    uart.puts("\n================================\n");
    uart.puts("  PlumOS Hybrid Kernel Booting...\n");
    uart.puts("  Architecture: ");
    uart.puts(ARCH);
    uart.puts("\n");
    uart.puts("  Version: 0.1.0\n");
    uart.puts("================================\n");

    if let Some(dt) = unsafe { DeviceTree::new(dtb_ptr) } {
        let regions: Vec<(usize, usize)> = dt.memory_regions().collect();
        init_buddy(&regions, PAGE_SIZE);
    } else {
        uart.puts("Failed to parse DTB! Using fallback.\n");
        let fallback = [(0x4000_0000, 0x800_0000)];
        init_buddy(&fallback, PAGE_SIZE);
    }

    init_heap(PAGE_SIZE);

    use itoa::Buffer;
    if let Some(page) = alloc_page() {
        uart.puts("Allocated page at: ");
        let mut buf = Buffer::new();
        let addr_str = buf.format(page);
        uart.puts(addr_str);
        uart.puts("\n");
    } else {
        uart.puts("Failed to allocate page!\n");
    }

    init::init_subsystems();
    uart.puts("Kernel booted successfully!\n");
    loop {}
}