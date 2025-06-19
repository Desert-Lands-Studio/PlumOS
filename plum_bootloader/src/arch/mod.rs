use uefi::boot::{self, AllocateType, MemoryType};

pub fn alloc_pages(addr: u64, size: u64) {
    let pages = (size + 0xFFF) / 0x1000;
    boot::allocate_pages(AllocateType::Address(addr), MemoryType::LOADER_DATA, pages as usize)
        .expect("Allocation failed");
}
