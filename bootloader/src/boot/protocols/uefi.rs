#![cfg(feature = "uefi")]

use uefi::prelude::*;
use uefi::proto::media::file::{File, FileAttribute, FileMode, FileInfo};
use uefi::boot::{self, AllocateType, MemoryType};
use uefi::CStr16;
use uefi::Status;

use crate::formats::plam::load_plam;

extern "C" {
    fn init_c();
}


const MAX_KERNEL_SIZE: usize = 16 * 1024 * 1024;

#[entry]
pub extern "efiapi" fn efi_main(
    image_handle: Handle,
    system_table: *mut SystemTable<Boot>,
) -> Status {
    unsafe { init_c(); }

    
    let st = unsafe { &mut *system_table };
    uefi::helpers::init(image_handle, st).expect("Failed to initialize UEFI");

    st.stdout().clear().unwrap();
    st.stdout().write_str("PlumOS UEFI Bootloader\n").unwrap();

    
    let fs_proto = boot::get_image_file_system(image_handle).unwrap();
    let mut fs = unsafe { &mut *fs_proto.get() };

    
    let mut root = fs.open_volume().unwrap();

    
    let kernel_path = CStr16::from_str_with_buf("kernel.plam", &mut [0u16; 32]).unwrap();
    let mut file = root.open(kernel_path, FileMode::Read, FileAttribute::empty()).unwrap();

    
    let info = file.get_boxed_info::<FileInfo>().unwrap();
    let size = info.file_size() as usize;
    if size > MAX_KERNEL_SIZE {
        st.stdout().write_str("Kernel too large!\n").unwrap();
        return Status::LOAD_ERROR;
    }

    
    let pages = (size + 4095) / 4096;
    let kernel_addr = boot::allocate_pages(
        AllocateType::AnyPages,
        MemoryType::LOADER_DATA,
        pages,
    ).unwrap().as_ptr() as usize;

    
    let mut buffer = [0u8; MAX_KERNEL_SIZE];
    let read_size = file.read(&mut buffer[..size]).unwrap();
    if read_size != size {
        return Status::LOAD_ERROR;
    }

    
    unsafe {
        core::ptr::copy_nonoverlapping(buffer.as_ptr(), kernel_addr as *mut u8, size);
    }

    
    let entry = load_plam(unsafe { core::slice::from_raw_parts(kernel_addr as *const u8, size) }, kernel_addr);

    
    let entry_fn: extern "C" fn() -> ! = unsafe { core::mem::transmute(entry) };
    entry_fn();
}