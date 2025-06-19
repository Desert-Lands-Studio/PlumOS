#![feature(alloc_error_handler)]
#![no_std]
#![no_main]

extern crate alloc;

use core::fmt::Write;
use uefi::prelude::*;
use uefi::{boot, fs::FileSystem, CString16};

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

    match fs.read(path.as_ref()) {
        Ok(buf) => {
            uefi::system::with_stdout(|stdout| {
                let _ = writeln!(stdout, "kernel.plam loaded: {} bytes", buf.len());
            });
            // For now just keep the file in memory to show success
            core::mem::forget(buf);
        }
        Err(_) => {
            uefi::system::with_stdout(|stdout| {
                let _ = writeln!(stdout, "failed to read kernel.plam");
            });
            return Status::LOAD_ERROR;
        }
    }

    Status::SUCCESS
}

#[alloc_error_handler]
fn oom(_: core::alloc::Layout) -> ! {
    loop {}
}
