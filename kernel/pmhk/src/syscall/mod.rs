pub mod ppm {
    /// Установка пакета
    pub fn install(package_name: *const u8, name_len: usize, options: &InstallOptions) -> i32 {
        // Проверка прав
        // Выделение памяти
        // Вызов менеджера пакетов
        0
    }
    
    /// Удаление пакета  
    pub fn remove(package_name: *const u8, name_len: usize) -> i32 {
        // ...
        0
    }
}

#[no_mangle]
pub extern "C" fn syscall_handler(syscall: usize, arg1: usize, arg2: usize, arg3: usize) -> usize {
    match syscall {
        1 => { // sys_write
            let buf = arg2 as *const u8;
            let len = arg3;
            if let Ok(s) = unsafe { core::slice::from_raw_parts(buf, len) } {
                if let Ok(s) = core::str::from_utf8(s) {
                    // Используйте уже существующий UART
                    let uart = crate::UART.lock();
                    uart.puts(s);
                }
            }
            len
        }
        0 => { // sys_read — заглушка
            0
        }
        60 => { // sys_exit
            loop {}
        }
        _ => 0,
    }
}