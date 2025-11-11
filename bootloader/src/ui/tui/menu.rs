pub struct BootMenu {
    items: [&'static str; 4],
    selected: usize,
    theme: Theme,
}

impl BootMenu {
    pub fn draw(&self) {
        
        
        
        
    }

    pub fn handle_key(&mut self, key: u8) {
        match key {
            b'j' | 0x51 => self.selected = (self.selected + 1) % self.items.len(), 
            b'k' | 0x50 => self.selected = self.selected.wrapping_sub(1),          
            b'\r' => self.boot_selected(),
            _ => {}
        }
    }
}