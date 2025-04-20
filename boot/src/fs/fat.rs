pub struct FileSystem {
    fs: fatfs::FileSystem<fatfs::StdIoWrapper<Disk>>,
}

impl FileSystem {
    pub fn new(lba_start: u64) -> Result<Self, FatError> {
        let disk = Disk::open(lba_start)?;
        let opts = fatfs::FsOptions::new();
        let fs = fatfs::FileSystem::new(disk, opts).map_err(|_| FatError::MountFailed)?;
        Ok(FileSystem { fs })
    }

    pub fn read_file(&self, path: &str) -> Result<Vec<u8>, FatError> {
        let root_dir = self.fs.root_dir();
        let file = root_dir.open_file(path).map_err(|_| FatError::NotFound)?;
        let size = file.len();
        let mut data = Vec::with_capacity(size as usize);
        // Читаем файл поблочно
        use fatfs::Read;
        let mut buf = [0u8; 1024];
        let mut f = file;
        loop {
            let bytes = f.read(&mut buf).unwrap_or(0);
            if bytes == 0 { break; }
            data.extend_from_slice(&buf[..bytes]);
        }
        Ok(data)
    }
}

// Представляет диск/раздел для fatfs. В режимах no_std вместо StdIoWrapper можно реализовать свои трейт, 
// здесь упрощенно:
struct Disk {
    start_lba: u64,
}
impl Disk {
    fn open(start: u64) -> Result<Self, FatError> { Ok(Disk { start_lba: start }) }
}
impl fatfs::IoBase for Disk {
    type Error = FatError;
}
impl fatfs::Read for Disk {
    type Error = FatError;
    fn read(&mut self, offs: u64, buf: &mut [u8]) -> Result<usize, FatError> {
        // Здесь нужно прочитать содержимое диска с offset.
        // offs = смещение от начала раздела в байтах.
        // Наш диск представляет раздел начиная с start_lba на физическом диске.
        let lba = self.start_lba + offs / 512;
        let count = (buf.len() + 511) / 512;
        unsafe {
            crate::utils::bios_disk_read(0x80, lba as u32, buf.as_mut_ptr(), count)?;
        }
        Ok(buf.len())
    }
}
