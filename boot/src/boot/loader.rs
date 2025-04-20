// src/boot/loaders.rs
pub enum BootError { FileNotFound, ReadError, InvalidKernel }

#[cfg(feature = "uefi")]
pub fn load_kernel(path: &str) -> Result<(), BootError> {
    use uefi::proto::media::file::{File, FileMode, FileType};

    // Получаем доступ к файловой системе EFI
    let sfs = unsafe { crate::uefi_services::system_table() }
        .boot_services()
        .locate_protocol::<SimpleFileSystem>()
        .unwrap();
    let mut root = unsafe { &mut *sfs.get() }.open_volume().unwrap();

    // Открываем файл ядра (.plam) на ESP разделе
    let handle = root.open(path, FileMode::Read, 0).map_err(|_| BootError::FileNotFound)?;
    let mut file = match handle.into_type().unwrap() {
        FileType::Regular(file) => file,
        _ => return Err(BootError::FileNotFound)
    };

    // Читаем файл целиком в выделенный буфер
    let file_info = file.get_info::<FileInfo>().unwrap();
    let size = file_info.file_size() as usize;
    let kernel_buffer = unsafe {
        // Аллоцируем страницы через BootServices (AllocatePool или AllocatePages)
        let bs = crate::uefi_services::system_table().boot_services();
        bs.allocate_pool(MemoryType::LOADER_DATA, size).unwrap()
    };
    let kernel_slice = unsafe { core::slice::from_raw_parts_mut(kernel_buffer, size) };
    file.read(kernel_slice).unwrap();

    // Получаем точку входа (предположим, формат .plam имеет в начале заголовок с адресом входа)
    let entry_addr = parse_plam_entry(kernel_slice).ok_or(BootError::InvalidKernel)?;

    // Вызываем функцию перехода к ядру
    unsafe { crate::boot::entry::enter_kernel(entry_addr) };
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

// Вспомогательная функция парсинга entry point из буфера ядра
fn parse_plam_entry(kernel_image: &[u8]) -> Option<*const u8> {
    // Предположим, формат .plam начинается с 64-байтового заголовка,
    // где смещение 0x10 содержит адрес точки входа (8 байт, little endian)
    if kernel_image.len() < 0x18 { return None; }
    let entry_addr = u64::from_le_bytes(kernel_image[0x10..0x18].try_into().unwrap());
    Some(entry_addr as *const u8)
}
