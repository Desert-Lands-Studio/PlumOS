#![no_std]
#![no_main]
#![feature(alloc_error_handler)]

extern crate alloc;

use alloc::vec::Vec;
use core::ptr::copy_nonoverlapping;
use uefi::{
    prelude::*,
    table::{boot::Boot, boot::{AllocateType, MemoryType}},
    proto::{
        loaded_image::LoadedImage,
        media::{
            file::{FileAttribute, FileMode},
            fs::SimpleFileSystem,
        },
    },
    CStr16, Handle, Status, SystemTable,
};
use log::info;
use plam::{header, sections, plam_reloc_t, PLAM_REL_64, PLAM_SEC_NOBITS};
mod arch;

#[entry]
fn efi_main(image_handle: Handle, mut system_table: SystemTable<Boot>) -> Status {
    // Initialize UEFI services
    system_table.uefi_services().init();
    let bs = system_table.boot_services();

    info!("PLUM Bootloader starting...");

    // Obtain the image handle for the loaded image protocol
    let loaded_image = bs.get_handle_for_protocol::<LoadedImage>()
        .expect("Failed to get loaded image protocol");
    let file_system = bs.get_handle_for_protocol::<SimpleFileSystem>()
        .expect("Failed to get file system protocol");
    
    // Access the file system and open kernel.plam
    let mut fs = unsafe { &mut *file_system.get_interface::<SimpleFileSystem>() };
    let mut root = fs.open_volume().expect("Failed to open volume");
    let mut file = root
        .open(cstr16!("kernel.plam"), FileMode::Read, FileAttribute::empty())
        .expect("Failed to open kernel.plam")
        .into_regular_file()
        .expect("Not a regular file");

    // Read the kernel file into a buffer
    let size = file.get_boxed_info::<uefi::proto::media::file::FileInfo>().unwrap().file_size() as usize;
    let mut buf = Vec::with_capacity(size);
    unsafe { buf.set_len(size); }
    file.read(&mut buf).expect("Failed to read kernel.plam");
    info!("Read kernel.plam: {} bytes", size);

    // Parse PLAM header
    let hdr = unsafe { header(&buf).expect("Invalid PLAM header") };
    if hdr.hdr_ver_major != 2 {
        panic!("Unsupported PLAM header version: {}.{}", hdr.hdr_ver_major, hdr.hdr_ver_minor);
    }
    info!(
        "PLAM header: type={:x}, cpu_id={:x}, entry_off={:x}",
        hdr.file_type, hdr.cpu_id, hdr.entry_off
    );

    // Load sections into memory
    const LOAD_BASE: u64 = 0x1000_0000;
    unsafe {
        for sec in sections(hdr, &buf) {
            if sec.size == 0 {
                continue;
            }
            if sec.flags & PLAM_SEC_NOBITS == 0 && (sec.offset as usize + sec.size as usize) > buf.len() {
                panic!("Section data out of bounds");
            }
            arch::alloc_pages(bs, LOAD_BASE + sec.addr, sec.size);
            if sec.flags & PLAM_SEC_NOBITS == 0 {
                copy_nonoverlapping(
                    buf.as_ptr().add(sec.offset as usize),
                    (LOAD_BASE + sec.addr) as *mut u8,
                    sec.size as usize,
                );
            } else {
                core::ptr::write_bytes((LOAD_BASE + sec.addr) as *mut u8, 0, sec.size as usize);
            }
            info!("Loaded section: addr={:x}, size={:x}", sec.addr, sec.size);
        }
    }

    // Apply relocations
    if hdr.reloc_count > 0 {
        let relocs = unsafe {
            core::slice::from_raw_parts(
                buf.as_ptr().add(hdr.reloc_table_off as usize) as *const plam_reloc_t,
                hdr.reloc_count as usize,
            )
        };
        for r in relocs {
            if r.type_ != PLAM_REL_64 {
                panic!("Unsupported relocation type: {}", r.type_);
            }
            unsafe {
                let p = (LOAD_BASE + r.offset) as *mut u64;
                *p = LOAD_BASE + r.addend as u64;
            }
            info!("Applied REL_64 at 0x{:x}", r.offset);
        }
    }

    // Prepare BootInfo and exit boot services
    let mmap_size = bs.memory_map_size();
    let mut mmap_buf = vec![0u8; mmap_size.map_size + 8 * mmap_size.entry_size];
    let (_, _) = system_table.exit_boot_services(image_handle, &mut mmap_buf)
        .expect("Failed to exit boot services");

    #[repr(C)]
    struct BootInfo {
        mmap_ptr: *const u8,
        mmap_entries: usize,
        kernel_base: u64,
        kernel_size: u64,
    }
    let info = BootInfo {
        mmap_ptr: mmap_buf.as_ptr(),
        mmap_entries: mmap_size.entry_count,
        kernel_base: LOAD_BASE,
        kernel_size: unsafe { sections(hdr, &buf).map(|s| s.size).sum() },
    };

    // Jump to kernel
    let entry = (LOAD_BASE + hdr.entry_off) as *const ();
    let kernel: extern "sysv64" fn(*const BootInfo) -> ! = unsafe { core::mem::transmute(entry) };
    kernel(&info);
}

#[alloc_error_handler]
fn oom(_: core::alloc::Layout) -> ! {
    loop {}
}