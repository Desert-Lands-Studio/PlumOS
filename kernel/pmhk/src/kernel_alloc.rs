use linked_list_allocator::LockedHeap;
use pmm::alloc_pages;
#[global_allocator]
static HEAP: LockedHeap = LockedHeap::empty();

pub fn init_heap(page_size: usize) {
    const HEAP_SIZE: usize = 16 * 1024 * 1024;
    let order = (HEAP_SIZE / page_size).ilog2() as usize;
    if let Some(start) = alloc_pages(order) {
        unsafe {
            HEAP.lock().init(start as *mut u8, HEAP_SIZE);
        }
    } else {
        panic!("Failed to allocate contiguous heap");
    }
}