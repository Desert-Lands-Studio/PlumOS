// src/tui/main.rs
#![no_std]
#![no_main]

mod panic;

#[cfg(feature = "uefi")]
use uefi::prelude::*;
#[cfg(feature = "uefi")]
use log::info;

#[cfg(feature = "uefi")]
#[entry]
fn uefi_main(image_handle: Handle, mut system_table: SystemTable<Boot>) -> Status {
    uefi_services::init(&mut system_table).unwrap();
    info!("Bootloader started in UEFI mode");

    if let Ok(config) = config::parser::load_config() {
        config::strings::select_locale(config.language.as_str());
        tui::screen::apply_theme(&config.theme);
    }

    tui::logo::print_logo();
    let choice = tui::menu::show_menu(); 

    match choice {
        BootChoice::Kernel(path) => {
            if let Err(e) = boot::loader::load_kernel(&path) {
                tui::screen::clear();
                tui::screen::write_at(0, 0, &format!("Ошибка загрузки ядра: {:?}", e));
                log::error!("Kernel load failed: {:?}", e); // Логирование
                system_table.boot_services().stall(5_000_000);
                return Status::LOAD_ERROR;
            }
        },
        BootChoice::Chain(os) => {
            if let Err(e) = boot::chainload::chainload_os(os) {
                tui::screen::clear();
                tui::screen::write_at(0, 0, &format!("Ошибка цепной загрузки: {:?}", e));
                system_table.boot_services().stall(5_000_000);
                return Status::LOAD_ERROR;
            }
        },
    }

    info!("Failed to load kernel or chainload. Halting.");
    system_table.boot_services().stall(5_000_000);
    Status::LOAD_ERROR
}

#[cfg(feature = "bios")]
#[no_mangle]
pub extern "C" fn _start() -> ! {
    tui::screen::init_vga_text_mode();

    if let Ok(config) = config::parser::load_config() {
        config::strings::select_locale(&config.language);
        tui::screen::apply_theme(&config.theme);
    }
    tui::logo::print_logo();
    let choice = tui::menu::show_menu();

    match choice {
        BootChoice::Kernel(path) => {
            if let Err(e) = boot::loader::load_kernel(&path) {
                tui::screen::clear();
                tui::screen::write_at(0, 0, &format!("Ошибка загрузки ядра: {:?}", e));
                utils::halt();
            }
        },
        BootChoice::Chain(os) => {
            if let Err(e) = boot::chainload::chainload_os(os) {
                tui::screen::clear();
                tui::screen::write_at(0, 0, &format!("Ошибка цепной загрузки: {:?}", e));
                utils::halt();
            }
        },
    }

    utils::reboot();
}
