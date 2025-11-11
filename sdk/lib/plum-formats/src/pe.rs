#[derive(Debug)]
pub struct Pe;

impl Pe {
    pub fn load(_data: &[u8]) -> Result<Self, &'static str> {
        Err("PE loader not implemented")
    }
}