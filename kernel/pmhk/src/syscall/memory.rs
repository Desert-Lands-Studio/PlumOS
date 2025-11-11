use crate::vm;

pub fn handle_mmap(
    addr: usize,
    length: usize,
    prot: usize,
    flags: usize,
    _fd: usize,
    _offset: usize,
) -> Result<usize, &'static str> {
    match vm::mmap(Some(addr), length, prot, flags) {
        Ok(vaddr) => Ok(vaddr),
        Err(_) => Err("Out of memory"),
    }
}