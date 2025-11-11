use plum_formats::plam::PlamHeader;
use crate::fs::fat32::Fat32Fs;

pub fn chain_load_kernel(fs: &Fat32Fs) -> ! {
    
    let kernel_data = fs.read_file("/boot/kernel.plam").expect("Kernel not found");

    
    let header = unsafe { &*(kernel_data.as_ptr() as *const PlamHeader) };
    if header.magic != 0x504C414D {
        panic!("Invalid PLAM magic");
    }

    
    
    let entry = (header.image_base + header.entry_offset) as *const ();
    let entry_fn: fn() -> ! = unsafe { core::mem::transmute(entry) };
    entry_fn();
}