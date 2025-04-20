use fatfs::{FileSystem as FatFs, FsOptions, Read};
use crate::utils::bios_disk_read;

pub struct FileSystem {
    fs: FatFs<fatfs::StdIoWrapper<Disk>>,
}

impl FileSystem {
    pub fn new(lba_start: u64) -> Result<Self, FatError> {
        let disk = Disk::open(lba_start)?;
        let opts = FsOptions::new();
        let fs = FatFs::new(disk, opts).map_err(|_| FatError::MountFailed)?;
        Ok(Self { fs })
    }

    pub fn read_file(&self, path: &str) -> Result<Vec<u8>, FatError> {
        let mut file = self.fs.root_dir().open_file(path).map_err(|_| FatError::NotFound)?;
        let mut data = Vec::new();
        file.read_to_end(&mut data).map_err(|_| FatError::ReadFailed)?;
        Ok(data)
    }
}

struct Disk {
    start_lba: u64,
}

impl Disk {
    fn open(start: u64) -> Result<Self, FatError> {
        Ok(Self { start_lba: start })
    }
}

impl fatfs::IoBase for Disk {
    type Error = FatError;
}

impl fatfs::Read for Disk {
    type Error = FatError;

    fn read(&mut self, offs: u64, buf: &mut [u8]) -> Result<usize, FatError> {
        let lba = self.start_lba + offs / 512;
        let count = (buf.len() + 511) / 512;
        bios_disk_read_safe(0x80, lba as u32, buf, count)?;
        Ok(buf.len())
    }
}

fn bios_disk_read_safe(drive: u8, lba: u32, buf: &mut [u8], sectors: usize) -> Result<(), FatError> {
    unsafe {
        crate::utils::bios_disk_read(drive, lba, buf.as_mut_ptr(), sectors)?;
    }
    Ok(())
}
