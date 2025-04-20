use ext2::Ext2;
use ext2::FileSystem as _;

pub struct Ext2FileSystem<D> {
    ext2: Ext2<D>,
}

impl Ext2FileSystem<Disk> {
    pub fn mount(lba_start: u64) -> Result<Self, Ext2Error> {
        let disk = Disk::open(lba_start)?;
        let ext2 = Ext2::new(disk).map_err(|_| Ext2Error::MountFailed)?;
        Ok(Self { ext2 })
    }

    pub fn read_file(&self, path: &str) -> Result<Vec<u8>, Ext2Error> {
        let mut file = self.ext2.open(path).map_err(|_| Ext2Error::NotFound)?;
        let mut data = vec![0u8; file.size() as usize];
        file.read_exact(&mut data).map_err(|_| Ext2Error::ReadFailed)?;
        Ok(data)
    }
}
