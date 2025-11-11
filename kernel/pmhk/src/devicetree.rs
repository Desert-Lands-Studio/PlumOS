use core::ffi::{c_char, CStr};
use core::str;
use core::mem;

#[repr(C)]
pub struct FdtHeader {
    magic: u32,
    totalsize: u32,
    off_dt_struct: u32,
    off_dt_strings: u32,
    off_mem_rsvmap: u32,
    version: u32,
    last_comp_version: u32,
    boot_cpuid_phys: u32,
    size_dt_strings: u32,
    size_dt_struct: u32,
}

#[repr(C)]
pub struct FdtProp {
    len: u32,
    nameoff: u32,
}

const FDT_MAGIC: u32 = 0xd00dfeed;

pub struct DeviceTree {
    base: *const u8,
    header: *const FdtHeader,
    prop_addr: usize,
}

impl DeviceTree {
    pub unsafe fn new(header_addr: usize) -> Option<Self> {
        let header = header_addr as *const FdtHeader;
        
        if (*header).magic != FDT_MAGIC {
            return None;
        }
        
        Some(DeviceTree {
            base: header_addr as *const u8,
            header,
            prop_addr: header_addr + (*header).off_dt_struct as usize,
        })
    }

    pub fn get_property(&self, name: &str) -> Option<&[u8]> {
        let mut prop_addr = self.prop_addr;
        
        unsafe {
            loop {
                let prop = &*(prop_addr as *const FdtProp);
                if prop.len == 0 {
                    return None;
                }
                
                let nameoff = u32::from_be(prop.nameoff);
                let prop_name_addr = (*self.header).off_dt_strings as usize + nameoff as usize;
                let cstr = CStr::from_ptr(prop_name_addr as *const c_char);
                
                if let Ok(prop_name) = cstr.to_str() {
                    if prop_name == name {
                        let prop_data_addr = prop_addr + mem::size_of::<FdtProp>();
                        return Some(core::slice::from_raw_parts(
                            prop_data_addr as *const u8, 
                            prop.len as usize
                        ));
                    }
                }
                
                prop_addr += mem::size_of::<FdtProp>() + prop.len as usize;
                prop_addr = (prop_addr + 3) & !3;
            }
        }
    }

    pub fn memory_regions(&self) -> MemoryRegionIter<'_> {
        MemoryRegionIter::new(self)
    }
}

pub struct MemoryRegionIter<'a> {
    dt: &'a DeviceTree,
    current: usize,
}

impl<'a> MemoryRegionIter<'a> {
    fn new(dt: &'a DeviceTree) -> Self {
        Self { dt, current: 0 }
    }
}

impl<'a> Iterator for MemoryRegionIter<'a> {
    type Item = (usize, usize);
    fn next(&mut self) -> Option<Self::Item> {
        if self.current == 0 {
            self.current += 1;
            Some((0x4000_0000, 0x800_0000))
        } else {
            None
        }
    }
}