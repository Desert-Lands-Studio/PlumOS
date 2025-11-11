use core::alloc::{GlobalAlloc, Layout};

pub struct PlumAllocator;

unsafe impl GlobalAlloc for PlumAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        
        extern "C" {
            fn pl_malloc(size: usize) -> *mut u8;
        }
        pl_malloc(layout.size())
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        extern "C" {
            fn pl_free(ptr: *mut u8);
        }
        pl_free(ptr)
    }
}

#[global_allocator]
static GLOBAL: PlumAllocator = PlumAllocator;