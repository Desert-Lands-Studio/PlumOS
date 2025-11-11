use crate::{BuddyAllocator, types::{TaskStruct, Inode, Socket}};

pub struct SlabAllocator;

impl SlabAllocator {
    pub const fn new() -> Self {
        Self
    }

    pub fn alloc_task_struct(&self, buddy: &BuddyAllocator) -> *mut TaskStruct {
        if let Some(page) = buddy.alloc_pages(0) {
            page as *mut TaskStruct
        } else {
            core::ptr::null_mut()
        }
    }

    pub fn free_task_struct(&self, buddy: &BuddyAllocator, ptr: *mut TaskStruct) {
        if !ptr.is_null() {
            buddy.free_pages(ptr as usize, 0);
        }
    }

    pub fn alloc_inode(&self, buddy: &BuddyAllocator) -> *mut Inode {
        if let Some(page) = buddy.alloc_pages(0) {
            page as *mut Inode
        } else {
            core::ptr::null_mut()
        }
    }

    pub fn alloc_socket(&self, buddy: &BuddyAllocator) -> *mut Socket {
        if let Some(page) = buddy.alloc_pages(0) {
            page as *mut Socket
        } else {
            core::ptr::null_mut()
        }
    }
}