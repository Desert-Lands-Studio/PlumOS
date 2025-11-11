pub struct Pl011Uart {
    base: usize,
}

impl Pl011Uart {
    pub const fn new(base: usize) -> Self {
        Self { base }
    }

    fn mmio_write(&self, reg: usize, val: u32) {
        unsafe { ((self.base + reg) as *mut u32).write_volatile(val) }
    }

    fn mmio_read(&self, reg: usize) -> u32 {
        unsafe { ((self.base + reg) as *const u32).read_volatile() }
    }

    pub fn init(&self) {
        
        self.mmio_write(0x30, 0x0000);

        
        self.mmio_write(0x24, 1); 
        self.mmio_write(0x28, 40); 

        
        self.mmio_write(0x2C, (1 << 4) | (3 << 5));

        
        self.mmio_write(0x30, (1 << 0) | (1 << 8) | (1 << 9));
    }

    pub fn putc(&self, c: u8) {
        
        while self.mmio_read(0x18) & (1 << 5) != 0 {}
        self.mmio_write(0x00, c as u32);
    }

    pub fn puts(&self, s: &str) {
        for b in s.bytes() {
            if b == b'\n' {
                self.putc(b'\r');
            }
            self.putc(b);
        }
    }
}
