use crate::Uart;
use crate::HalResult;

pub struct Ns16550Uart {
    base: usize,
}

impl Ns16550Uart {
    pub const unsafe fn new(base: usize) -> Self {
        Self { base }
    }

    fn outb(&self, reg: usize, value: u8) {
        unsafe {
            ((self.base + reg) as *mut u8).write_volatile(value);
        }
    }

    fn inb(&self, reg: usize) -> u8 {
        unsafe {
            ((self.base + reg) as *const u8).read_volatile()
        }
    }
}

impl Uart for Ns16550Uart {
    fn init(&mut self) -> HalResult<()> {
        self.outb(1, 0x00); // Disable interrupts
        self.outb(3, 0x80); // Enable DLAB
        self.outb(0, 0x03); // Baud rate divisor low
        self.outb(1, 0x00); // Baud rate divisor high
        self.outb(3, 0x03); // 8N1, disable DLAB
        self.outb(2, 0xC7); // Enable FIFO
        Ok(())
    }

    fn putc(&mut self, c: u8) -> HalResult<()> {
        while self.inb(5) & 0x20 == 0 {}
        self.outb(0, c);
        Ok(())
    }

    fn getc(&mut self) -> HalResult<Option<u8>> {
        if self.inb(5) & 0x01 != 0 {
            Ok(Some(self.inb(0)))
        } else {
            Ok(None)
        }
    }
}