use crate::devicetree::DeviceTree;
use pmm::init_buddy;
use crate::config::config::PAGE_SIZE;

pub unsafe fn init_platform(dtb_ptr: usize) {
    if let Some(dt) = DeviceTree::new(dtb_ptr) {
        let mut regions_array = [(0usize, 0usize); 8];
        let mut count = 0;
        for region in dt.memory_regions() {
            if count < regions_array.len() {
                regions_array[count] = region;
                count += 1;
            } else {
                break;
            }
        }
        let regions_slice = if count == 0 {
            &[(0x4000_0000usize, 0x800_0000usize)]
        } else {
            &regions_array[..count]
        };

        init_buddy(regions_slice, PAGE_SIZE);
    }
}
