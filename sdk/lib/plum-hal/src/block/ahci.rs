use crate::block::{BlockDevice, BlockError};

pub struct AhciController {
    base: usize,
}

impl AhciController {
    pub const unsafe fn new(base: usize) -> Self {
        Self { base }
    }
    fn mmio_read(&self, reg: usize) -> u32 {
        unsafe { ((self.base + reg) as *const u32).read_volatile() }
    }
    fn mmio_write(&self, reg: usize, value: u32) {
        unsafe { ((self.base + reg) as *mut u32).write_volatile(value); }
    }
    pub fn init(&mut self) -> Result<(), &'static str> {
        let cap = self.mmio_read(0x00);
        if cap & (1 << 31) == 0 {
            return Err("Controller does not support AHCI");
        }
        let ghc = self.mmio_read(0x04);
        self.mmio_write(0x04, ghc | (1 << 31));
        log::info!("AHCI Controller initialized");
        Ok(())
    }
    pub fn detect_ports(&self) -> u32 {
        let pi = self.mmio_read(0x0C);
        log::debug!("AHCI Ports implemented: {:032b}", pi);
        pi
    }
}

impl BlockDevice for AhciController {
    fn read_blocks(&mut self, _lba: u64, _buffer: &mut [u8]) -> Result<(), BlockError> {
        Err(BlockError::IoError)
    }
    fn write_blocks(&mut self, _lba: u64, _buffer: &[u8]) -> Result<(), BlockError> {
        Err(BlockError::IoError)
    }
    fn block_size(&self) -> usize { 512 }
    fn block_count(&self) -> u64 { 0 }
}