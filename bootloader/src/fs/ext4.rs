pub struct Ext4Fs {
    
}

impl Ext4Fs {
    pub fn read_file(&self, _path: &str) -> Result<&'static [u8], ()> {
        Ok(&[])
    }
}