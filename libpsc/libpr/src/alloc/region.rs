pub struct Region {
    
}

impl Region {
    pub fn new(size: usize) -> Option<Self> {
        unsafe {
            extern "C" {
                fn pl_region_create(size: usize) -> *mut c_void;
            }
            let region = pl_region_create(size);
            if region.is_null() {
                None
            } else {
                Some(Region { inner: region })
            }
        }
    }

    pub fn alloc<T>(&mut self) -> Option<&mut T> {
    }
}