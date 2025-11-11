use crate::hal::Uart;

pub struct Uart16550 {
    base: usize,
}

impl Uart16550 {
    pub const unsafe fn new(base: usize) -> Self {
        Self { base }
    }
    
    fn mmio_write(&self, reg: usize, value: u8) {
        unsafe { (self.base + reg) as *mut u8 }.write_volatile(value);
    }
    
    fn mmio_read(&self, reg: usize) -> u8 {
        unsafe { (self.base + reg) as *const u8 }.read_volatile()
    }
}

impl Uart for Uart16550 {
    fn init(&mut self) {
        
        self.mmio_write(1, 0x00); 
        self.mmio_write(3, 0x80); 
        self.mmio_write(0, 0x03); 
        self.mmio_write(1, 0x00);
        self.mmio_write(3, 0x03);
        self.mmio_write(2, 0xC7);
    }
    
    fn putc(&mut self, c: u8) {
        while self.mmio_read(5) & 0x20 == 0 {}
        self.mmio_write(0, c);
    }
    
    fn getc(&mut self) -> Option<u8> {
        if self.mmio_read(5) & 0x01 != 0 {
            Some(self.mmio_read(0))
        } else {
            None
        }
    }
}