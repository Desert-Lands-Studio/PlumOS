#![no_std]

pub mod allocator;
pub mod types;
pub mod arch;

pub use allocator::buddy::BuddyAllocator;
pub use allocator::slab::SlabAllocator;
pub use arch::Arch;

pub const FALLBACK_MEMORY_REGION: (usize, usize) = (0x4000_0000, 0x800_0000); 

mod mm {
    use super::{BuddyAllocator, SlabAllocator};
    use spin::Once;
    use core::sync::atomic::{AtomicBool, Ordering};
    use crate::types::TaskStruct;

    static BUDDY_ALLOCATOR: Once<BuddyAllocator> = Once::new();
    static SLAB_ALLOCATOR: Once<SlabAllocator> = Once::new();
    static BUDDY_INITED: AtomicBool = AtomicBool::new(false);
    static SLAB_INITED: AtomicBool = AtomicBool::new(false);

    pub fn init_buddy(regions: &[(usize, usize)], page_size: usize) {
        if BUDDY_INITED.swap(true, Ordering::Relaxed) {
            panic!("init_buddy() called twice!");
        }
        let mut allocator = BuddyAllocator::new();
        allocator.init(regions, page_size);
        BUDDY_ALLOCATOR.call_once(|| allocator);
    }

    pub fn init_slab() {
        if SLAB_INITED.swap(true, Ordering::Relaxed) {
            panic!("init_slab() called twice!");
        }
        SLAB_ALLOCATOR.call_once(|| SlabAllocator::new());
    }

    pub fn alloc_task_struct() -> *mut u8 {
        SLAB_ALLOCATOR
            .get()
            .map(|slab| slab.alloc_task_struct(&BUDDY_ALLOCATOR.get().unwrap()) as *mut u8)
            .unwrap_or(core::ptr::null_mut())
    }

    pub fn free_task_struct(ptr: *mut u8) {
        if let Some(slab) = SLAB_ALLOCATOR.get() {
            slab.free_task_struct(&BUDDY_ALLOCATOR.get().unwrap(), ptr as *mut TaskStruct);
        }
    }

    pub fn alloc_page() -> Option<usize> {
        BUDDY_ALLOCATOR.get().and_then(|b| b.alloc_pages(0))
    }

    pub fn free_page(addr: usize) {
        if let Some(b) = BUDDY_ALLOCATOR.get() {
            b.free_pages(addr, 0);
        }
    }

    pub fn alloc_pages(order: usize) -> Option<usize> {
    BUDDY_ALLOCATOR.get().and_then(|b| b.alloc_pages(order))
}
}

pub use mm::{init_buddy, init_slab, alloc_task_struct, free_task_struct, alloc_page, free_page, alloc_pages};