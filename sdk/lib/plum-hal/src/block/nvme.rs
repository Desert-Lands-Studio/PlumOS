use crate::block::{BlockDevice, BlockError};

pub struct NvmeController {
    base: usize,
}

impl NvmeController {
    pub const unsafe fn new(base: usize) -> Self {
        Self { base }
    }
    fn mmio_read(&self, reg: usize) -> u32 {
        unsafe { ((self.base + reg) as *const u32).read_volatile() }
    }
    #[allow(dead_code)]
    fn mmio_write(&self, reg: usize, value: u32) {
        unsafe { ((self.base + reg) as *mut u32).write_volatile(value); }
    }
    pub fn init(&mut self) -> Result<(), &'static str> {
        let vs = self.mmio_read(0x08);
        log::info!("NVMe Version: {}.{}", (vs >> 16) & 0xffff, vs & 0xffff);
        log::info!("NVMe Controller initialized");
        Ok(())
    }
}

impl BlockDevice for NvmeController {
    fn read_blocks(&mut self, _lba: u64, _buffer: &mut [u8]) -> Result<(), BlockError> {
        Err(BlockError::IoError)
    }
    fn write_blocks(&mut self, _lba: u64, _buffer: &[u8]) -> Result<(), BlockError> {
        Err(BlockError::IoError)
    }
    fn block_size(&self) -> usize { 512 }
    fn block_count(&self) -> u64 { 0 }
}