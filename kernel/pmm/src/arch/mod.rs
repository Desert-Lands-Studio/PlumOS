pub trait Arch {
    const PAGE_SIZE: usize;
    const PHYS_OFFSET: usize;
    unsafe fn phys_to_virt(phys: usize) -> usize;
    unsafe fn invalidate(virt: usize);
    unsafe fn set_table(table: usize);
}