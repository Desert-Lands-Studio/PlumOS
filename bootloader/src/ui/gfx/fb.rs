pub struct Framebuffer {
    base: *mut u8,
    width: u32,
    height: u32,
    stride: u32,
    bpp: u8,
}

impl Framebuffer {
    pub fn clear(&mut self, color: u32) { ... }
    pub fn draw_bitmap(&mut self, x: u32, y: u32, bmp: &Bitmap) { ... }
}