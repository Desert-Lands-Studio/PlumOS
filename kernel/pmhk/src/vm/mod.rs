use crate::config::config::PAGE_SIZE;
use pmm::alloc_page;
use core::result::Result;

#[derive(Debug)]
pub enum VmError {
    OutOfMemory,
    InvalidAddress,
}

pub fn mmap(
    vaddr_hint: Option<usize>,
    length: usize,
    _prot: usize,
    _flags: usize,
) -> Result<usize, VmError> {
    let pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    let vaddr = vaddr_hint.unwrap_or(0x1000_0000); 

    for _ in 0..pages {
        if alloc_page().is_none() {
            return Err(VmError::OutOfMemory);
        }
    }

    Ok(vaddr)
}