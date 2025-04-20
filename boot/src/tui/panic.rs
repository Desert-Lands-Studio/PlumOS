use core::panic::PanicInfo;

#[cfg(feature = "uefi")]
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    use log::error;

    error!("==== SYSTEM PANIC ====");
    error!("{:?}", info);

    loop {}
}

#[cfg(feature = "bios")]
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    use crate::tui::screen::{self, Color};
    use crate::utils;

    // Установить цвет текста: белый текст на красном фоне
    screen::set_color(Color::White, Color::Red);
    screen::clear();

    // Заголовок
    screen::write_at(0, 0, "==== SYSTEM PANIC ====");

    // Выводим причину паники
    screen::write_at(0, 2, "Reason:");

    let panic_message = format!("{:?}", info);
    let mut line = 4;
    for part in panic_message.as_bytes().chunks(80) {
        if let Ok(text) = core::str::from_utf8(part) {
            screen::write_at(0, line, text);
            line += 1;
        }
    }

    // Сообщение о перезагрузке
    screen::write_at(0, line + 2, "Rebooting in 10 seconds...");

    // Небольшая задержка
    for _ in 0..10 {
        utils::sleep(1_000_000); // например, ожидание 1 секунды через hlt+loop
    }

    utils::reboot();
}
