#[derive(Debug)]
pub struct Coff;

impl Coff {
    pub fn load(_data: &[u8]) -> Result<Self, &'static str> {
        Err("COFF loader not implemented")
    }
}