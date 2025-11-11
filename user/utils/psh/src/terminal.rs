#[derive(Debug, Clone, Copy)]
pub enum Color {
    Black, Red, Green, Yellow, Blue, Magenta, Cyan, White, RGB(u8, u8, u8),
}

#[derive(Debug, Clone, Copy)]
pub enum Style {
    Normal, Bold, Italic, Underline,
}

pub struct Terminal;

impl Terminal {
    pub fn new() -> Self {
        Self
    }

    pub fn print(&self, text: &str, color: Color, style: Style) {
        if cfg!(target_os = "none") {
            print!("{}", text);
            return;
        }

        let color_code = match color {
            Color::Black => "30",
            Color::Red => "31",
            Color::Green => "32",
            Color::Yellow => "33",
            Color::Blue => "34",
            Color::Magenta => "35",
            Color::Cyan => "36",
            Color::White => "37",
            Color::RGB(r, g, b) => {
                self.print_rgb(text, r, g, b, style);
                return;
            }
        };

        let style_code = match style {
            Style::Normal => "0",
            Style::Bold => "1",
            Style::Italic => "3",
            Style::Underline => "4",
        };

        print!("\x1B[{};{}m{}\x1B[0m", style_code, color_code, text);
    }

    pub fn println(&self, text: &str, color: Color, style: Style) {
        self.print(text, color, style);
        println!();
    }

    fn print_rgb(&self, text: &str, r: u8, g: u8, b: u8, style: Style) {
        let style_code = match style {
            Style::Normal => "0",
            Style::Bold => "1",
            Style::Italic => "3",
            Style::Underline => "4",
        };
        print!("\x1B[{};2;{};{};{}m{}\x1B[0m", style_code, r, g, b, text);
    }
}