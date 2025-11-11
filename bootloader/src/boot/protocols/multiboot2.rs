pub struct MultibootInfo {
    
}

pub fn parse_multiboot(magic: u32, _addr: usize) -> Option<MultibootInfo> {
    if magic != 0x36d76289 {
        None
    } else {
        
        Some(MultibootInfo {})
    }
}