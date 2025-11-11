pub struct Fat32Fs {
    
}

impl Fat32Fs {
    pub fn read_file(&self, _path: &str) -> Result<&'static [u8], ()> {
        Ok(&[])
    }
}