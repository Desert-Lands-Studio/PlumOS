#[cfg(feature = "uefi")]
use uefi::proto::console::text::Key;
#[cfg(feature = "uefi")]
use uefi::table::boot::{EventType, TimerTrigger, TPL_APPLICATION};
use crate::config::parser::{get_entries, get_default_index, get_timeout, BootEntry};
use crate::tui::screen;
use crate::utils;

pub enum BootChoice {
    Kernel(String),
    Chain(OS),
}

#[cfg(feature = "uefi")]
pub fn show_menu() -> BootChoice {
    let st = unsafe { crate::uefi_services::system_table() };
    let bs = st.boot_services();
    let entries = get_entries();
    let mut index = get_default_index();
    let timeout = get_timeout().unwrap_or(10); // секунды
    let step = 100_000; // 0.1 секунды в микросекундах
    let mut elapsed = 0;
    let timer_event = bs.create_event(EventType::TIMER, TPL_APPLICATION, None, None).unwrap();
    bs.set_timer(timer_event, TimerTrigger::Relative(step)).unwrap();
    loop {
        screen::clear();
        screen::write_at(0, 0, crate::config::strings::s().choose_os);
        for (i, entry) in entries.iter().enumerate() {
            let marker = if i == index { ">" } else { " " };
            let name = match entry {
                BootEntry::Kernel { name, .. } => name,
                BootEntry::Chain { name, .. } => name,
            };
            screen::write_at(2, 2 + i, &format!("{} {}", marker, name));
        }
        let key_event = st.stdin().wait_for_key_event();
        let events = [key_event, timer_event];
        let event_index = bs.wait_for_event(&events).unwrap();
        if event_index == 0 {
            if let Ok(Some(key)) = st.stdin().read_key() {
                match key {
                    Key::Special(SpecialKey::Up) => {
                        if index > 0 {
                            index -= 1;
                        }
                    },
                    Key::Special(SpecialKey::Down) => {
                        if index < entries.len() - 1 {
                            index += 1;
                        }
                    },
                    Key::Special(SpecialKey::Enter) => {
                        let chosen = &entries[index];
                        return match chosen {
                            BootEntry::Kernel { path, .. } => BootChoice::Kernel(path.clone()),
                            BootEntry::Chain { os, .. } => BootChoice::Chain(os.clone()),
                        };
                    },
                    _ => {},
                }
            }
        } else {
            elapsed += step;
            if elapsed / 1_000_000 >= timeout {
                let chosen = &entries[index];
                return match chosen {
                    BootEntry::Kernel { path, .. } => BootChoice::Kernel(path.clone()),
                    BootEntry::Chain { os, .. } => BootChoice::Chain(os.clone()),
                };
            }
            bs.set_timer(timer_event, TimerTrigger::Relative(step)).unwrap();
        }
    }
}

#[cfg(feature = "bios")]
pub fn show_menu() -> BootChoice {
    let entries = get_entries();
    let mut index = get_default_index();
    loop {
        screen::clear();
        screen::write_at(0, 0, crate::config::strings::s().choose_os);
        for (i, entry) in entries.iter().enumerate() {
            let marker = if i == index { ">" } else { " " };
            let name = match entry {
                BootEntry::Kernel { name, .. } => name,
                BootEntry::Chain { name, .. } => name,
            };
            screen::write_at(2, 2 + i, &format!("{} {}", marker, name));
        }
        let key = utils::read_key(); // Убедитесь, что utils::read_key() реализован для BIOS
        match key {
            Key::ArrowUp => {
                if index > 0 {
                    index -= 1;
                }
            },
            Key::ArrowDown => {
                if index < entries.len() - 1 {
                    index += 1;
                }
            },
            Key::Enter => {
                let chosen = &entries[index];
                return match chosen {
                    BootEntry::Kernel { path, .. } => BootChoice::Kernel(path.clone()),
                    BootEntry::Chain { os, .. } => BootChoice::Chain(os.clone()),
                };
            },
            _ => {},
        }
    }
}