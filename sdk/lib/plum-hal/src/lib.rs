#![no_std]

pub mod uart;
pub mod block;

#[derive(Debug, Clone, Copy)]
pub enum HalError {
    DeviceNotFound,
    Timeout,
    InvalidParameter,
    NotSupported,
    IoError,
}

impl From<block::BlockError> for HalError {
    fn from(error: block::BlockError) -> Self {
        match error {
            block::BlockError::DeviceError => HalError::DeviceNotFound,
            block::BlockError::Timeout => HalError::Timeout,
            block::BlockError::InvalidParameter => HalError::InvalidParameter,
            block::BlockError::NotSupported => HalError::NotSupported,
            block::BlockError::IoError => HalError::IoError,
        }
    }
}

impl From<&'static str> for HalError {
    fn from(_: &'static str) -> Self {
        HalError::IoError
    }
}

pub type HalResult<T> = Result<T, HalError>;

pub trait Uart {
    fn init(&mut self) -> HalResult<()>;
    fn putc(&mut self, c: u8) -> HalResult<()>;
    fn getc(&mut self) -> HalResult<Option<u8>>;
    fn puts(&mut self, s: &str) -> HalResult<()> {
        for &b in s.as_bytes() {
            self.putc(b)?;
        }
        Ok(())
    }
    fn write(&mut self, data: &[u8]) -> HalResult<()> {
        for &byte in data {
            self.putc(byte)?;
        }
        Ok(())
    }
    fn read(&mut self, buffer: &mut [u8]) -> HalResult<usize> {
        let mut count = 0;
        for byte in buffer.iter_mut() {
            match self.getc()? {
                Some(b) => {
                    *byte = b;
                    count += 1;
                }
                None => break,
            }
        }
        Ok(count)
    }
}

pub trait Timer {
    fn init(&mut self) -> HalResult<()>;
    fn set_interval(&mut self, interval_ns: u64) -> HalResult<()>;
    fn get_counter(&self) -> HalResult<u64>;
    fn enable(&mut self) -> HalResult<()>;
    fn disable(&mut self) -> HalResult<()>;
}

#[macro_export]
macro_rules! debug_print {
    ($($arg:tt)*) => {
        #[cfg(feature = "debug")]
        {
            use crate::hal::Uart;
            if let Some(uart) = crate::hal::uart::get_default_uart() {
                let _ = uart.lock().puts(&format!($($arg)*));
            }
        }
    };
}

#[macro_export]
macro_rules! debug_println {
    ($($arg:tt)*) => {
        $crate::debug_print!($($arg)*);
        $crate::debug_print!("\n");
    };
}

pub fn init_hal() -> HalResult<()> {
     #[cfg(all(
        any(target_arch = "x86_64", target_arch = "riscv64"),
        not(feature = "uefi")
    ))]
    crate::uart::init_default_uart()?;

    crate::block::init_block_devices()?;
    Ok(())
}

pub mod memory {
    use core::ptr;

    pub fn memset(dest: *mut u8, value: u8, count: usize) {
        unsafe {
            ptr::write_bytes(dest, value, count);
        }
    }

    pub fn memcpy(dest: *mut u8, src: *const u8, count: usize) {
        unsafe {
            ptr::copy_nonoverlapping(src, dest, count);
        }
    }

    pub fn memmove(dest: *mut u8, src: *const u8, count: usize) {
        unsafe {
            ptr::copy(src, dest, count);
        }
    }

    pub fn memcmp(s1: *const u8, s2: *const u8, count: usize) -> i32 {
        for i in 0..count {
            unsafe {
                let a = *s1.add(i);
                let b = *s2.add(i);
                if a != b {
                    return a as i32 - b as i32;
                }
            }
        }
        0
    }
}