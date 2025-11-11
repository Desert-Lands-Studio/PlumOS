pub mod ppm {
    pub fn install(package_name: *const u8, name_len: usize, options: &InstallOptions) -> i32 {
        0
    }
    
    pub fn remove(package_name: *const u8, name_len: usize) -> i32 {
        0
    }
}

#[no_mangle]
pub extern "C" fn syscall_handler(syscall: usize, arg1: usize, arg2: usize, arg3: usize) -> usize {
    match syscall {
        1 => { 
            let buf = arg2 as *const u8;
            let len = arg3;
            if let Ok(s) = unsafe { core::slice::from_raw_parts(buf, len) } {
                if let Ok(s) = core::str::from_utf8(s) {
                    let uart = crate::UART.lock();
                    uart.puts(s);
                }
            }
            len
        }
        0 => { 
            0
        }
        60 => { 
            loop {}
        }
        _ => 0,
    }
}