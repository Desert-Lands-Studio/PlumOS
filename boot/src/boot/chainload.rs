// src/boot/chainload.rs
pub enum BootError { FileNotFound, ReadError, InvalidKernel }

#[cfg(feature = "uefi")]
pub fn load_kernel(path: &str) -> Result<(), BootError> {
    use uefi::proto::media::file::{File, FileMode, FileType};
    use uefi::table::boot::MemoryType;

    let sfs = unsafe { crate::uefi_services::system_table() }
        .boot_services()
        .locate_protocol::<SimpleFileSystem>()
        .unwrap();
    let mut root = unsafe { &mut *sfs.get() }.open_volume().unwrap();

    let handle = root.open(path, FileMode::Read, 0).map_err(|_| BootError::FileNotFound)?;
    let mut file = match handle.into_type().unwrap() {
        FileType::Regular(file) => file,
        _ => return Err(BootError::FileNotFound),
    };

    let file_info = file.get_info::<FileInfo>().unwrap();
    let size = file_info.file_size() as usize;
    let kernel_buffer = unsafe {
        let bs = crate::uefi_services::system_table().boot_services();
        bs.allocate_pool(MemoryType::LOADER_DATA, size).unwrap()
    };
    let kernel_slice = unsafe { core::slice::from_raw_parts_mut(kernel_buffer, size) };
    file.read(kernel_slice).unwrap();

    let entry_offset = parse_plam_entry(kernel_slice).ok_or(BootError::InvalidKernel)?;
    let entry_addr = (kernel_buffer as u64) + entry_offset;
    unsafe { crate::boot::entry::jump_to_entry(entry_addr) };
    Ok(())
}

#[cfg(feature = "bios")]
pub fn load_kernel(path: &str) -> Result<(), BootError> {
    // В BIOS режиме у нас нет готовой файловой системы.
    // Используем модули fs::fat или fs::ext2 напрямую.
    // Например, сначала определим на каком разделе искать файл:
    let partitions = bootsector::list_partitions().unwrap();
    let part = partitions.into_iter()
        .find(|p| p.is_active())  // условно: или по метке, или по типу
        .ok_or(BootError::FileNotFound)?;
    if part.filesystem == Filesystem::FAT32 {
        let fs = fs::fat::FileSystem::new(part.start_lba)?;
        let file_data = fs.read_file(path).map_err(|_| BootError::FileNotFound)?;
        let entry_addr = parse_plam_entry(&file_data).ok_or(BootError::InvalidKernel)?;
        unsafe { crate::boot::entry::enter_kernel(entry_addr) };
    } else if part.filesystem == Filesystem::EXT2 {
        let fs = fs::ext2::FileSystem::mount(part.start_lba)?;
        let file_data = fs.read_file(path).map_err(|_| BootError::FileNotFound)?;
        let entry_addr = parse_plam_entry(&file_data).ok_or(BootError::InvalidKernel)?;
        unsafe { crate::boot::entry::enter_kernel(entry_addr) };
    } else {
        return Err(BootError::FileNotFound);
    }
    Ok(())
}

const PLAM_MAGIC: u32 = 0x504C4D32; // "PLM2"
const PLAM_ARCH_X86_64: u32 = 0x8664;
const PLAM_ARCH_ARM64: u32 = 0xAA64;

fn parse_plam_entry(kernel_image: &[u8]) -> Option<u64> {
    if kernel_image.len() < 120 {
        return None;
    }
    let magic = u32::from_le_bytes(kernel_image[0..4].try_into().unwrap());
    if magic != PLAM_MAGIC {
        return None;
    }
    let target_arch = u32::from_le_bytes(kernel_image[20..24].try_into().unwrap());
    #[cfg(target_arch = "x86_64")]
    if target_arch != PLAM_ARCH_X86_64 {
        return None;
    }
    #[cfg(target_arch = "aarch64")]
    if target_arch != PLAM_ARCH_ARM64 {
        return None;
    }
    let entry_point = u64::from_le_bytes(kernel_image[8..16].try_into().unwrap());
    if entry_point as usize >= kernel_image.len() {
        return None;
    }
    Some(entry_point)
}