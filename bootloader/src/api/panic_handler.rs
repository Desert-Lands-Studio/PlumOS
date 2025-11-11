use core::panic::PanicInfo;

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    crate::hal::Console::puts("KERNEL PANIC: ");
    if let Some(location) = info.location() {
        crate::hal::Console::puts(location.file());
        crate::hal::Console::puts(":");
        crate::hal::Console::puts(itoa::Buffer::new().format(location.line()));
    }
    crate::hal::Console::puts("\n");

    if let Some(_args) = info.message().as_str() {
        use core::fmt::Write;
        let mut writer = crate::hal::console::ConsoleWriter;
        let _ = write!(&mut writer, "{}", info.message());
    }

    crate::halt();
}