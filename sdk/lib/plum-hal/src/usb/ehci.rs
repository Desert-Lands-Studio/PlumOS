pub struct Ehci {
    base: usize,
}

impl Ehci {
    pub fn new(base: usize) -> Self {
        Ehci { base }
    }

    pub fn init(&self) {
        
    }
}