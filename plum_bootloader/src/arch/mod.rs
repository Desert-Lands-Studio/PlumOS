use uefi::table::boot::{AllocateType, MemoryType, BootServices};

pub fn alloc_pages(bs: &BootServices, addr: u64, size: u64) {
    let pages = (size + 0xFFF) / 0x1000;
    bs.allocate_pages(
        AllocateType::Address(addr),
        MemoryType::LOADER_DATA,
        pages as usize,
    ).expect("Allocation failed");
}