use core::ptr;
use spin::Mutex;

pub struct StackAllocator {
    start: usize,
    end: usize,
    current: usize,
}

impl StackAllocator {
    pub const fn new(start: usize, size: usize) -> Self {
        Self {
            start,
            end: start + size,
            current: start,
        }
    }

    pub fn allocate(&mut self, size: usize, align: usize) -> Option<*mut u8> {
        let aligned_addr = (self.current + align - 1) & !(align - 1);
        if aligned_addr + size <= self.end {
            let ptr = aligned_addr as *mut u8;
            self.current = aligned_addr + size;
            Some(ptr)
        } else {
            None
        }
    }

    pub fn allocate_zeroed(&mut self, size: usize, align: usize) -> Option<*mut u8> {
        self.allocate(size, align).map(|ptr| {
            unsafe { ptr::write_bytes(ptr, 0, size) };
            ptr
        })
    }

    pub fn reset(&mut self) {
        self.current = self.start;
    }

    pub fn used(&self) -> usize {
        self.current - self.start
    }

    pub fn available(&self) -> usize {
        self.end - self.current
    }
}

pub static STACK_ALLOC: Mutex<StackAllocator> =
    Mutex::new(StackAllocator::new(0x4100_0000, 64 * 1024));
