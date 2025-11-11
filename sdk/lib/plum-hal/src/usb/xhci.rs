pub struct Xhci {
    base: usize,
}

impl Xhci {
    pub fn new(base: usize) -> Self {
        Xhci { base }
    }

    pub fn init(&self) {
        
    }
}