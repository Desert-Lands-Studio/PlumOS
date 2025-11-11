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
    // Выравниваем длину до размера страницы
    let pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    let vaddr = vaddr_hint.unwrap_or(0x1000_0000); // TODO: выделение из VMA

    // Заглушка: маппинг пока не реализован — просто проверяем, что можем выделить физ. память
    for _ in 0..pages {
        if alloc_page().is_none() {
            return Err(VmError::OutOfMemory);
        }
    }

    Ok(vaddr)
}