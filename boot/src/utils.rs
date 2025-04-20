// src/utils.rs
#![allow(dead_code)]
use crc::{Crc, CRC_32_ISO_HDLC};
use uuid::Uuid;

pub fn compute_crc32(data: &[u8]) -> u32 {
    const CRC32: Crc<u32> = Crc::<u32>::new(&CRC_32_ISO_HDLC);
    CRC32.checksum(data)
}

pub fn uuid_to_string(uuid: &[u8; 16]) -> String {
    Uuid::from_bytes(*uuid).to_string()
}

pub fn halt() -> ! {
    loop {
        unsafe {
            core::arch::asm!("hlt", options(nomem, nostack, preserves_flags));
        }
    }
}

pub fn reboot() -> ! {
    unsafe {
        // Разрешить клавиатурный контроллер
        outb(0x64, 0xFE);
        halt(); // На случай если перезагрузка не сработает сразу
    }
}

pub fn sleep(microseconds: u64) {
    // Приблизительное ожидание через HLT в цикле
    for _ in 0..(microseconds / 100) {
        unsafe {
            core::arch::asm!("hlt", options(nomem, nostack, preserves_flags));
        }
    }
}

/// Пишет байт в порт ввода/вывода
unsafe fn outb(port: u16, value: u8) {
    core::arch::asm!(
        "out dx, al",
        in("dx") port,
        in("al") value,
        options(nostack, preserves_flags),
    );
}