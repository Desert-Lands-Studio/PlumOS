// src/tui/screen.rs
#![allow(dead_code)]
use core::ptr::write_volatile;

#[cfg(feature = "bios")]
const VGA_BUFFER: *mut u8 = 0xb8000 as *mut u8;

pub fn init_vga_text_mode() {
    #[cfg(feature = "bios")]
    unsafe {
        // Настраиваем VGA в режим 80x25 текста, цвет по умолчанию.
        // Предположим, что bootloader это уже сделал; если нет, нужно послать команду портам CRTC.
    }
}

pub fn clear() {
    #[cfg(feature = "uefi")]
    {
        use uefi::proto::console::text::Output;
        let _ = unsafe { crate::uefi_services::system_table() }.stdout().clear();
    }
    #[cfg(feature = "bios")]
    {
        for i in 0..(80*25) {
            unsafe {
                *VGA_BUFFER.add(i*2) = b' ';
                *VGA_BUFFER.add(i*2 + 1) = 0x07; // атрибут серый на черном (по умолчанию)
            }
        }
    }
}

pub fn set_color(fg: u8, bg: u8) {
    #[cfg(feature = "uefi")]
    {
        use uefi::proto::console::text::{Color, Output};
        let con_out = unsafe { crate::uefi_services::system_table() }.stdout();
        let _ = con_out.set_color(Color::from_efi(fg), Color::from_efi(bg));
    }
    #[cfg(feature = "bios")]
    {
        // Можно сохранить текущий цвет в статическую переменную для использования при выводе
        unsafe {
            CURRENT_COLOR = ((bg & 0xF) << 4) | (fg & 0xF);
        }
    }
}

#[cfg(feature = "bios")]
static mut CURRENT_COLOR: u8 = 0x07;

pub fn write_at(x: usize, y: usize, text: &str) {
    #[cfg(feature = "uefi")]
    {
        let con_out = unsafe { crate::uefi_services::system_table() }.stdout();
        // UEFI не имеет прямой функции поставить курсор и напечатать строку, 
        // но можно использовать .set_cursor_position + .output_string:
        let _ = con_out.set_cursor_position(x, y);
        let _ = con_out.output_string(text);
    }
    #[cfg(feature = "bios")]
    {
        let mut offset = (y * 80 + x) * 2;
        unsafe {
            for &byte in text.as_bytes() {
                if offset >= 80*25*2 { break; }
                *VGA_BUFFER.add(offset) = byte;
                *VGA_BUFFER.add(offset + 1) = CURRENT_COLOR;
                offset += 2;
            }
        }
    }
}
