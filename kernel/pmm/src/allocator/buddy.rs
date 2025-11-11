use spin::Mutex;

const MAX_ORDER: usize = 10;
const MAX_FREE_BLOCKS: usize = 1024;

pub struct BuddyList {
    count: usize,
    blocks: [usize; MAX_FREE_BLOCKS],
}

impl BuddyList {
    pub const fn new() -> Self {
        Self { count: 0, blocks: [0; MAX_FREE_BLOCKS] }
    }

    fn push(&mut self, addr: usize) {
        if self.count < MAX_FREE_BLOCKS {
            self.blocks[self.count] = addr;
            self.count += 1;
        }
    }

    fn pop(&mut self) -> Option<usize> {
        if self.count == 0 {
            return None;
        }
        self.count -= 1;
        Some(self.blocks[self.count])
    }

    fn len(&self) -> usize {
        self.count
    }

    fn clear(&mut self) {
        self.count = 0;
    }

    fn iter(&self) -> &[usize] {
        &self.blocks[..self.count]
    }

    fn position(&self, val: usize) -> Option<usize> {
        self.iter()
            .iter()
            .position(|&a| a == val)
    }

    fn swap_remove(&mut self, idx: usize) {
        if idx < self.count {
            self.count -= 1;
            self.blocks[idx] = self.blocks[self.count];
        }
    }
}

pub struct BuddyAllocator {
    free_lists: [Mutex<BuddyList>; MAX_ORDER],
    memory_start: usize,
    memory_end: usize,
    page_size: usize, // ← новое поле
    initialized: bool,
}

impl BuddyAllocator {
    pub const fn new() -> Self {
        const EMPTY: Mutex<BuddyList> = Mutex::new(BuddyList::new());
        Self {
            free_lists: [EMPTY; MAX_ORDER],
            memory_start: 0,
            memory_end: 0,
            page_size: 4096, // временно
            initialized: false,
        }
    }

    pub fn init(&mut self, regions: &[(usize, usize)], page_size: usize) {
        self.page_size = page_size;
        if regions.is_empty() {
            return;
        }
        let (start, size) = regions[0];
        self.memory_start = start;
        self.memory_end = start + size;

        let aligned_start = (start + page_size - 1) & !(page_size - 1);
        let aligned_end = self.memory_end & !(page_size - 1);
        if aligned_start >= aligned_end {
            return;
        }

        let total_pages = (aligned_end - aligned_start) / page_size;
        let mut list0 = self.free_lists[0].lock();
        for i in 0..total_pages {
            list0.push(aligned_start + i * page_size);
        }
        drop(list0);
        self.initialized = true;
        self.coalesce_free_blocks();
    }

    pub fn page_size(&self) -> usize {
        self.page_size
    }

    fn coalesce_free_blocks(&self) {
        for order in 1..MAX_ORDER {
            let mut curr = [0; MAX_FREE_BLOCKS];
            let curr_len = {
                let guard = self.free_lists[order - 1].lock();
                let len = guard.len();
                curr[..len].copy_from_slice(guard.iter());
                len
            };
            curr[..curr_len].sort_unstable();
            self.free_lists[order - 1].lock().clear();

            let mut i = 0;
            while i < curr_len {
                if i + 1 < curr_len {
                    let a = curr[i];
                    let b = curr[i + 1];
                    let block_size = self.page_size * (1 << (order - 1));
                    if Self::are_buddies(a, b, block_size) {
                        self.free_lists[order].lock().push(a.min(b));
                        i += 2;
                        continue;
                    }
                }
                self.free_lists[order - 1].lock().push(curr[i]);
                i += 1;
            }
        }
    }

    fn are_buddies(a: usize, b: usize, block_size: usize) -> bool {
        let base = a.min(b);
        let buddy = base ^ block_size;
        buddy == a.max(b)
    }

    pub fn alloc_pages(&self, order: usize) -> Option<usize> {
        if !self.initialized || order >= MAX_ORDER {
            return None;
        }
        if let Some(block) = self.free_lists[order].lock().pop() {
            return Some(block);
        }
        for higher in order + 1..MAX_ORDER {
            if let Some(large) = self.alloc_pages(higher) {
                return self.split_block(large, higher, order);
            }
        }
        None
    }

    fn split_block(&self, block: usize, from_order: usize, to_order: usize) -> Option<usize> {
        let current = block;
        let mut current_order = from_order;
        while current_order > to_order {
            current_order -= 1;
            let block_size = self.page_size * (1 << current_order);
            self.free_lists[current_order].lock().push(current + block_size);
        }
        Some(current)
    }

    pub fn free_pages(&self, page: usize, order: usize) {
        if !self.initialized || order >= MAX_ORDER {
            return;
        }
        let mut current_page = page;
        let mut current_order = order;
        while current_order < MAX_ORDER - 1 {
            let block_size = self.page_size * (1 << current_order);
            let buddy = current_page ^ block_size;
            let mut list = self.free_lists[current_order].lock();
            if let Some(pos) = list.position(buddy) {
                list.swap_remove(pos);
                current_page = current_page.min(buddy);
                current_order += 1;
                continue;
            } else {
                list.push(current_page);
                return;
            }
        }
        self.free_lists[current_order].lock().push(current_page);
    }

    pub fn alloc_contiguous_pages(&self, count: usize) -> Option<usize> {
        fn log2_ceil(mut n: usize) -> usize {
            let mut log = 0;
            n = n.saturating_sub(1);
            while n > 0 {
                n >>= 1;
                log += 1;
            }
            log
        }

        let order = log2_ceil(count);
        if 1usize << order < count {
            // нужно больше, чем 2^order — увеличить
            // но для простоты пока требуем степень двойки
            return None;
        }

        self.alloc_pages(order)
    }
}
