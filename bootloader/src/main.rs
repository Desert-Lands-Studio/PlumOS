#![no_std]
#![no_main]
#![feature(alloc_error_handler)]

mod alloc;
mod features;

use core::fmt::Write;
use core::ptr;

// === Архитектурно-условные импорты PLAM ===
#[cfg(target_arch = "x86_64")]
use plum_formats::plam::{PlamHeader, PLAM_MAGIC, PLAM_CPU_X86_64};
#[cfg(target_arch = "aarch64")]
use plum_formats::plam::{PlamHeader, PLAM_MAGIC, PLAM_CPU_ARM64};
#[cfg(target_arch = "riscv64")]
use plum_formats::plam::{PlamHeader, PLAM_MAGIC, PLAM_CPU_RISCV64};

// === Зависимости (только для baremetal) ===
#[cfg(not(feature = "uefi"))]
use {
    plum_hal::block::{BlockError, init_block_devices, get_block_device_manager, BlockDevice},
    spin::Mutex,
    crate::alloc::early_alloc::STACK_ALLOC,
    core::alloc::{GlobalAlloc, Layout},
    plum_hal::Uart,
};

// === Архитектурно-условный UART ===
#[cfg(target_arch = "x86_64")]
#[cfg(not(feature = "uefi"))]
use plum_hal::uart::ns16550::Ns16550Uart as DefaultUart;

#[cfg(target_arch = "aarch64")]
#[cfg(not(feature = "uefi"))]
use plum_hal::uart::pl011::Pl011Uart as DefaultUart;

#[cfg(target_arch = "riscv64")]
#[cfg(not(feature = "uefi"))]
use plum_hal::uart::uart16550::Uart16550 as DefaultUart;

// === Конфигурация UART ===
#[cfg(target_arch = "aarch64")]
const UART_BASE: usize = 0x0900_0000;

#[cfg(target_arch = "x86_64")]
const UART_BASE: usize = 0x3F8;

#[cfg(target_arch = "riscv64")]
const UART_BASE: usize = 0x1000_0000;

// === Глобальный аллокатор (только baremetal) ===
#[cfg(not(feature = "uefi"))]
struct EarlyAllocator;

#[cfg(not(feature = "uefi"))]
unsafe impl GlobalAlloc for EarlyAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        STACK_ALLOC
            .lock()
            .allocate(layout.size(), layout.align())
            .unwrap_or(ptr::null_mut())
    }
    unsafe fn dealloc(&self, _ptr: *mut u8, _layout: Layout) {}
}

#[cfg(not(feature = "uefi"))]
#[global_allocator]
static EARLY_ALLOC: EarlyAllocator = EarlyAllocator;

// === UART (динамическая инициализация, только baremetal) ===
#[cfg(not(feature = "uefi"))]
use spin::Once;

#[cfg(not(feature = "uefi"))]
static UART: Once<Mutex<DefaultUart>> = Once::new();

// === Консольный вывод ===
pub struct ConsoleWriter;
impl Write for ConsoleWriter {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        #[cfg(feature = "uefi")]
        {
            // В UEFI — заглушка. Реализуется в uefi.rs через `uefi::println!`
            Ok(())
        }
        #[cfg(not(feature = "uefi"))]
        {
            let mut uart = UART.get().expect("UART not initialized").lock(); // ← MUTABLE!
            for b in s.bytes() {
                let _ = uart.putc(b);
            }
            Ok(())
        }
    }
}

// === Обработчики ошибок ===
#[panic_handler]
fn panic(info: &core::panic::PanicInfo) -> ! {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "PANIC: {}\n", info);
    loop {}
}

#[cfg(not(feature = "uefi"))]
#[alloc_error_handler]
fn alloc_error_handler(layout: Layout) -> ! {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "Allocation error: {:?}\n", layout);
    loop {}
}

// === Инициализация UART (baremetal) ===
#[cfg(not(feature = "uefi"))]
fn init_uart() {
    UART.call_once(|| {
        let mut uart = unsafe { DefaultUart::new(UART_BASE) };
        let _ = uart.init();
        Mutex::new(uart)
    });
}

// === Загрузка ядра с диска (baremetal) ===
#[cfg(not(feature = "uefi"))]
pub fn load_kernel_from_disk() -> Result<usize, BlockError> {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: initializing block devices...\n");
    init_block_devices()?;
    let mut mgr = get_block_device_manager();
    let dev = mgr.get_device(0).ok_or(BlockError::DeviceError)?;
    let _ = write!(&mut w, "bootloader: reading kernel from LBA\n");

    const KERNEL_SIZE: usize = 32 * 1024 * 1024;
    // Используем выделение через STACK_ALLOC вместо rust_alloc::vec!
    let mut buffer = {
        let ptr = STACK_ALLOC
            .lock()
            .allocate(KERNEL_SIZE, 1)
            .unwrap_or(ptr::null_mut());
        unsafe { core::slice::from_raw_parts_mut(ptr, KERNEL_SIZE) }
    };

    dev.read_blocks(2048 / 512, buffer)?;
    if buffer.len() < core::mem::size_of::<PlamHeader>() {
        return Err(BlockError::InvalidParameter);
    }

    let header = unsafe { &*(buffer.as_ptr() as *const PlamHeader) };
    if header.magic != PLAM_MAGIC {
        return Err(BlockError::InvalidParameter);
    }

    #[cfg(target_arch = "x86_64")]
    if header.cpu_id != PLAM_CPU_X86_64 {
        return Err(BlockError::InvalidParameter);
    }
    #[cfg(target_arch = "aarch64")]
    if header.cpu_id != PLAM_CPU_ARM64 {
        return Err(BlockError::InvalidParameter);
    }
    #[cfg(target_arch = "riscv64")]
    if header.cpu_id != PLAM_CPU_RISCV64 {
        return Err(BlockError::InvalidParameter);
    }

    let image_base = header.image_base as usize;
    let entry_offset = header.entry_offset as usize;
    let entry = image_base + entry_offset;

    unsafe {
        ptr::copy_nonoverlapping(buffer.as_ptr(), image_base as *mut u8, buffer.len());
    }

    let _ = write!(&mut w, "bootloader: PLAM OK, loaded to {:#x}, entry {:#x}\n", image_base, entry);
    Ok(entry)
}

// === Точка входа ===
#[no_mangle]
pub extern "C" fn bootloader_main() -> ! {
    #[cfg(not(feature = "uefi"))]
    {
        init_uart();

        let mut w = ConsoleWriter;
        #[cfg(target_arch = "x86_64")]
        let _ = write!(&mut w, "x86_64 Baremetal Bootloader Initialized!\n");
        #[cfg(target_arch = "aarch64")]
        let _ = write!(&mut w, "AArch64 Baremetal Bootloader Initialized!\n");
        #[cfg(target_arch = "riscv64")]
        let _ = write!(&mut w, "RISC-V64 Baremetal Bootloader Initialized!\n");

        match load_kernel_from_disk() {
            Ok(entry) => {
                let _ = write!(&mut w, "bootloader: jumping to kernel at {:#x}\n", entry);
                let entry_fn: extern "C" fn() -> ! = unsafe { core::mem::transmute(entry) };
                entry_fn();
            }
            Err(e) => {
                let _ = write!(&mut w, "bootloader: failed to load kernel: {:?}\n", e);
                loop {}
            }
        }
    }

    #[cfg(feature = "uefi")]
    {
        // Реализуется в отдельном файле `src/boot/protocols/uefi.rs`
        // Здесь — заглушка, чтобы не паниковать
        loop {}
    }
}