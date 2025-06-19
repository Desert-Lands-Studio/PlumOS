#![feature(alloc_error_handler)]
#![no_std]
#![no_main]

extern crate alloc;

use core::fmt::Write;
use uefi::prelude::*;
use uefi::{boot, fs::FileSystem, CString16};
use plam::{self, PLAM_SEC_NOBITS, PLAM_SEC_EXEC, PLAM_SEC_READ, PLAM_SEC_WRITE};
mod arch;

#[entry]
fn efi_main() -> Status {
    // Print a simple message
    uefi::system::with_stdout(|stdout| {
        let _ = writeln!(stdout, "PLUM Bootloader starting...");
    });

    // Access the file system of the loaded image
    let fs_proto = match boot::get_image_file_system(boot::image_handle()) {
        Ok(p) => p,
        Err(_) => return Status::LOAD_ERROR,
    };
    let mut fs = FileSystem::new(fs_proto);

    let path = match CString16::try_from("kernel.plam") {
        Ok(p) => p,
        Err(_) => return Status::LOAD_ERROR,
    };

    let buf = match fs.read(path.as_ref()) {
        Ok(b) => {
            uefi::system::with_stdout(|stdout| {
                let _ = writeln!(stdout, "kernel.plam loaded: {} bytes", b.len());
            });
            b
        }
        Err(_) => {
            uefi::system::with_stdout(|stdout| {
                let _ = writeln!(stdout, "failed to read kernel.plam");
            });
            return Status::LOAD_ERROR;
        }
         };

    // Parse PLAM header
    let hdr = match unsafe { plam::header(&buf) } {
        Some(h) => h,
        None => {
            uefi::system::with_stdout(|stdout| {
                let _ = writeln!(stdout, "invalid PLAM header");
            });
            return Status::LOAD_ERROR;
        }
    };

    // Load sections
    for sec in unsafe { plam::sections(hdr, &buf) } {
        // Only load allocatable sections
        let flags = sec.flags as u32;
        if flags & (PLAM_SEC_EXEC | PLAM_SEC_READ | PLAM_SEC_WRITE | PLAM_SEC_NOBITS) == 0 {
            continue;
        }
        let addr = sec.addr as u64;
        let size = sec.size as u64;
        arch::alloc_pages(addr, size);

        unsafe {
            let dest = addr as *mut u8;
            if flags & PLAM_SEC_NOBITS != 0 {
                core::ptr::write_bytes(dest, 0, size as usize);
            } else {
                let src_off = sec.offset as usize;
                let src = &buf[src_off..src_off + sec.size as usize];
                core::ptr::copy_nonoverlapping(src.as_ptr(), dest, sec.size as usize);
            }
        }
    }

        // Jump to kernel entry
    let entry: extern "C" fn() -> ! = unsafe { core::mem::transmute(hdr.entry_off as usize) };
    entry()
}

#[alloc_error_handler]
fn oom(_: core::alloc::Layout) -> ! {
    loop {}
}
