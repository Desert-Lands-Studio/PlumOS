#[derive(Debug)]
pub struct Elf {
    pub entry_offset: usize,
}

impl Elf {
    pub fn load(_data: &[u8]) -> Result<Self, &'static str> {
        Ok(Self { entry_offset: 0 })
    }

    pub fn alloc_and_load(&self, _base: usize) -> Result<*mut u8, &'static str> {
        unimplemented!("ELF loader not implemented")
    }
}