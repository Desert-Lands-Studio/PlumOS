pub mod ahci;
pub mod nvme;
pub mod sdhci;

pub use ahci::AhciController;
pub use nvme::NvmeController;
pub use sdhci::SdhciController;

use spin::Mutex;


#[derive(Debug)]
pub enum BlockError {
    DeviceError,
    Timeout,
    InvalidParameter,
    NotSupported,
    IoError,
}

impl From<&'static str> for BlockError {
    fn from(_: &'static str) -> Self {
        BlockError::IoError
    }
}

pub type BlockResult<T> = Result<T, BlockError>;

pub trait BlockDevice {
    fn read_blocks(&mut self, lba: u64, buffer: &mut [u8]) -> BlockResult<()>;
    fn write_blocks(&mut self, lba: u64, buffer: &[u8]) -> BlockResult<()>;
    fn block_size(&self) -> usize;
    fn block_count(&self) -> u64;
    fn supports_dma(&self) -> bool { false }
    fn flush(&mut self) -> BlockResult<()> { Ok(()) }
}

pub enum BlockDeviceType {
    Ahci(AhciController),
    Nvme(NvmeController),
    SdCard(SdhciController),
}

impl BlockDevice for BlockDeviceType {
    fn read_blocks(&mut self, lba: u64, buffer: &mut [u8]) -> BlockResult<()> {
        match self {
            BlockDeviceType::Ahci(dev) => dev.read_blocks(lba, buffer),
            BlockDeviceType::Nvme(dev) => dev.read_blocks(lba, buffer),
            BlockDeviceType::SdCard(dev) => dev.read_blocks(lba, buffer),
        }
    }
    fn write_blocks(&mut self, lba: u64, buffer: &[u8]) -> BlockResult<()> {
        match self {
            BlockDeviceType::Ahci(dev) => dev.write_blocks(lba, buffer),
            BlockDeviceType::Nvme(dev) => dev.write_blocks(lba, buffer),
            BlockDeviceType::SdCard(dev) => dev.write_blocks(lba, buffer),
        }
    }
    fn block_size(&self) -> usize {
        match self {
            BlockDeviceType::Ahci(dev) => dev.block_size(),
            BlockDeviceType::Nvme(dev) => dev.block_size(),
            BlockDeviceType::SdCard(dev) => dev.block_size(),
        }
    }
    fn block_count(&self) -> u64 {
        match self {
            BlockDeviceType::Ahci(dev) => dev.block_count(),
            BlockDeviceType::Nvme(dev) => dev.block_count(),
            BlockDeviceType::SdCard(dev) => dev.block_count(),
        }
    }
    fn supports_dma(&self) -> bool {
        match self {
            BlockDeviceType::Ahci(dev) => dev.supports_dma(),
            BlockDeviceType::Nvme(dev) => dev.supports_dma(),
            BlockDeviceType::SdCard(dev) => dev.supports_dma(),
        }
    }
    fn flush(&mut self) -> BlockResult<()> {
        match self {
            BlockDeviceType::Ahci(dev) => dev.flush(),
            BlockDeviceType::Nvme(dev) => dev.flush(),
            BlockDeviceType::SdCard(dev) => dev.flush(),
        }
    }
}

pub struct BlockDeviceManager {
    devices: [Option<BlockDeviceType>; 8],
    count: usize,
}

impl BlockDeviceManager {
    pub const fn new() -> Self {
        Self {
            devices: [
                None, None, None, None,
                None, None, None, None,
            ],
            count: 0,
        }
    }
    pub fn add_device(&mut self, device: BlockDeviceType) -> BlockResult<()> {
        if self.count >= self.devices.len() {
            return Err(BlockError::NotSupported);
        }
        if device.block_count() == 0 {
            return Err(BlockError::DeviceError);
        }
        self.devices[self.count] = Some(device);
        self.count += 1;
        Ok(())
    }
    pub fn get_device(&mut self, index: usize) -> Option<&mut BlockDeviceType> {
        if index < self.count {
            self.devices[index].as_mut()
        } else {
            None
        }
    }
    pub fn get_device_by_name(&mut self, _name: &str) -> Option<&mut BlockDeviceType> {
        // Исправлено: _name вместо name
        self.devices[0..self.count].iter_mut().find_map(|dev| dev.as_mut())
    }
    pub fn device_count(&self) -> usize {
        self.count
    }
    pub fn total_capacity(&self) -> u64 {
        let mut total = 0;
        for i in 0..self.count {
            if let Some(dev) = &self.devices[i] {
                total += dev.block_count() * dev.block_size() as u64;
            }
        }
        total
    }
}

static BLOCK_DEVICE_MANAGER: Mutex<BlockDeviceManager> = Mutex::new(BlockDeviceManager::new());

pub fn init_block_devices() -> BlockResult<()> {
    let mut mgr = BLOCK_DEVICE_MANAGER.lock();
    log::info!("Initializing block devices...");
    #[cfg(target_arch = "x86_64")]
    {
        log::info!("Detecting AHCI controllers...");
        let mut ahci = unsafe { AhciController::new(0x400000) };
        if ahci.init().is_ok() {
            if let Err(e) = mgr.add_device(BlockDeviceType::Ahci(ahci)) {
                log::warn!("Failed to add AHCI device: {:?}", e);
            } else {
                log::info!("AHCI controller initialized");
            }
        }
    }
    #[cfg(target_arch = "aarch64")]
    {
        log::info!("Detecting SDHCI controllers...");
        let mut sdhci = SdhciController::new(0x7E200000);
        if sdhci.init().is_ok() {
            if let Err(e) = mgr.add_device(BlockDeviceType::SdCard(sdhci)) {
                log::warn!("Failed to add SD card device: {:?}", e);
            } else {
                log::info!("SDHCI controller initialized");
            }
        }
    }
    #[cfg(target_arch = "riscv64")]
    {
        log::info!("Detecting NVMe controllers...");
        let mut nvme = unsafe { NvmeController::new(0x30000000) };
        if nvme.init().is_ok() {
            if let Err(e) = mgr.add_device(BlockDeviceType::Nvme(nvme)) {
                log::warn!("Failed to add NVMe device: {:?}", e);
            } else {
                log::info!("NVMe controller initialized");
            }
        }
    }
    log::info!("Block devices initialized: {}", mgr.device_count());
    log::info!("Total storage capacity: {} bytes", mgr.total_capacity());
    if mgr.device_count() == 0 {
        log::warn!("No block devices found!");
        return Err(BlockError::DeviceError);
    }
    Ok(())
}

pub fn get_block_device_manager() -> spin::MutexGuard<'static, BlockDeviceManager> {
    BLOCK_DEVICE_MANAGER.lock()
}

// Утилиты для работы с разделами
pub mod partition {
    use super::*;

    #[repr(C, packed)]
    #[derive(Debug, Copy, Clone)]
    pub struct MbrPartition {
        pub status: u8,
        pub chs_start: [u8; 3],
        pub type_code: u8,
        pub chs_end: [u8; 3],
        pub lba_start: u32,
        pub sector_count: u32,
    }

    #[repr(C, packed)]
    #[derive(Debug, Copy, Clone)] // Добавлено Copy
    pub struct Mbr {
        pub bootstrap: [u8; 446],
        pub partitions: [MbrPartition; 4],
        pub signature: u16,
    }

    pub fn read_mbr(device: &mut dyn BlockDevice) -> BlockResult<Mbr> {
        let mut mbr_data = [0u8; 512];
        device.read_blocks(0, &mut mbr_data)?;
        unsafe {
            let mbr = &*(mbr_data.as_ptr() as *const Mbr);
            if mbr.signature != 0xAA55 {
                return Err(BlockError::InvalidParameter);
            }
            Ok(*mbr) // Теперь можно, т.к. Mbr: Copy
        }
    }

    pub fn is_valid_partition(partition: &MbrPartition) -> bool {
        partition.type_code != 0 && partition.sector_count > 0
    }
}