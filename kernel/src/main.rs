#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    let vga_buffer = 0xb8000 as *mut u8;
    let msg = b"PlumKernel booted successfully!";
    for (i, byte) in msg.iter().enumerate() {
        unsafe {
            core::ptr::write_volatile(vga_buffer.add(i * 2), *byte);
            core::ptr::write_volatile(vga_buffer.add(i * 2 + 1), 0x0F);
        }
    }
    loop {}
}

#[unsafe(no_mangle)]
pub extern "C" fn _start() -> ! {
    loop {}
}

