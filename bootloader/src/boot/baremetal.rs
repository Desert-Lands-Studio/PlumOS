#![no_std]
#![no_main]
#![feature(alloc_error_handler)]

use core::fmt::Write;
use core::ptr;

use plum_formats::plam::{PlamHeader, PLAM_MAGIC}; // предполагаем, что в crate plum_formats есть plam
use plum_hal::console::ConsoleWriter; // используем ConsoleWriter из plum-hal

/// Адрес, по которому ожидается PLAM-заголовок.
/// Подставь реальный адрес для своей платформы.
const PLAM_LOAD_ADDR: usize = 0x8000_0000usize; // пример — поменяй под свою систему

/// Максимально допустимый размер загружаемого ядра (предохранитель).
const MAX_KERNEL_SIZE: usize = 32 * 1024 * 1024; // 32 MiB, пример

/// Публичная точка для загрузки ядра.
/// По умолчанию пытается загрузить PLAM; позже сюда можно добавить распознавание формата.
pub fn load_kernel() -> Result<usize, &'static str> {
    // Сейчас просто делегируем реализации PLAM.
    match load_kernel_plam() {
        Ok(addr) => Ok(addr),
        Err(e) => {
            // Сообщаем причину и возвращаем ошибку.
            let mut w = ConsoleWriter;
            let _ = write!(&mut w, "bootloader: plam loader failed: {}\n", e);
            // можно попробовать другие форматы здесь в будущем:
            // load_kernel_elf() or others.
            Err(e)
        }
    }
}

/// Реализация загрузки PLAM-образа.
/// Простая валидация заголовка и возврат адреса стартового сектора/ядра.
fn load_kernel_plam() -> Result<usize, &'static str> {
    // Логируем начало.
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: attempting to load .plam from {:#x}\n", PLAM_LOAD_ADDR);

    // Мы читаем заголовок из памяти по PLAM_LOAD_ADDR.
    // Поскольку работа с сырыми указателями — unsafe, оборачиваем.
    unsafe {
        let header_ptr = PLAM_LOAD_ADDR as *const PlamHeader;

        // Пытаемся прочитать header (последует UB, если адрес неверный).
        // Здесь делаем минимальную проверку: читаем magic и cpu id/size.
        let header = ptr::read_unaligned(header_ptr);

        // Проверка магического числа
        if header.magic != PLAM_MAGIC {
            let _ = write!(
                &mut w,
                "bootloader: plam magic mismatch: found {:#x}, expected {:#x}\n",
                header.magic, PLAM_MAGIC
            );
            return Err("invalid plam magic");
        }

        // Проверка архитектуры (для AArch64)
        if header.cpu != PLAM_CPU_ARM64 {
            let _ = write!(
                &mut w,
                "bootloader: plam cpu mismatch: found {:#x}, expected {:#x}\n",
                header.cpu, PLAM_CPU_ARM64
            );
            return Err("invalid plam cpu arch");
        }

        // Некоторое базовое ограничение на размер
        if header.totalsize == 0 || (header.totalsize as usize) > MAX_KERNEL_SIZE {
            let _ = write!(
                &mut w,
                "bootloader: plam image size invalid: {}\n",
                header.totalsize
            );
            return Err("invalid plam image size");
        }

        // Проверка адреса загрузки/смещения (используем entry_offset из заголовка)
        let entry_offset = header.entry_offset as usize;
        let image_base = header.image_base as usize;
        let entry_addr = image_base.checked_add(entry_offset).ok_or("entry addr overflow")?;

        // Здесь можно добавить валидацию (checksum, signature, etc.)
        // Также — декомпрессия/релокация, если требуется (stub пока).
        // Предполагаем, что образ уже в памяти и готов.

        let _ = write!(
            &mut w,
            "bootloader: plam header ok; entry at {:#x}, size {}\n",
            entry_addr, header.totalsize
        );

        Ok(entry_addr)
    }
}

/// Заглушка: COFF
fn load_kernel_coff() -> Result<usize, &'static str> {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: COFF loader not implemented\n");
    Err("coff loader not implemented")
}

/// Заглушка: PE
fn load_kernel_pe() -> Result<usize, &'static str> {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: PE loader not implemented\n");
    Err("pe loader not implemented")
}

/// Заглушка: ELF
fn load_kernel_elf() -> Result<usize, &'static str> {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: ELF loader not implemented\n");
    Err("elf loader not implemented")
}

/// Заглушка: Mach-O
fn load_kernel_macho() -> Result<usize, &'static str> {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: Mach-O loader not implemented\n");
    Err("macho loader not implemented")
}

/// Утилиты для hal/logger — если понадобится, здесь можно добавить вспомогательные функции.
/// Например, функцию `halt` (diverging), которую можно вызывать при Фатальной ошибке.
#[allow(dead_code)]
fn halt() -> ! {
    let mut w = ConsoleWriter;
    let _ = write!(&mut w, "bootloader: halt\n");
    loop {
        // Ничего не делаем — остаёмся в бесконечном цикле.
        core::hint::spin_loop();
    }
}