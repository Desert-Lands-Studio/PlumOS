pub fn init() {
    unsafe { outb(0x64, 0xAE); }
}

pub fn read_key() -> Option<u8> {
    if unsafe { inb(0x64) } & 1 != 0 {
        Some(unsafe { inb(0x60) })
    } else {
        None
    }
}