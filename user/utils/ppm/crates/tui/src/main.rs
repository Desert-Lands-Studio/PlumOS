use crossterm::{
    event::{self, Event, KeyCode},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Direction, Layout},
    style::{Color, Style},
    widgets::{Block, Borders, List, ListItem, ListState, Paragraph},
    Frame, Terminal,
};
use std::io;

struct TuiApp {
    list_state: ListState,
    items: Vec<String>,
}

impl TuiApp {
    fn new() -> Self {
        let items = vec![
            "📥 Install Package".to_string(),
            "🗑️ Remove Package".to_string(),
            "🔄 Update Packages".to_string(),
            "🔍 Search Packages".to_string(),
            "📋 List Installed".to_string(),
            "📄 Package Info".to_string(),
            "🎛️ Change Channel".to_string(),
            "🧹 Clean Cache".to_string(),
        ];
        Self {
            list_state: ListState::default(),
            items,
        }
    }

    fn next(&mut self) {
        let i = match self.list_state.selected() {
            Some(i) => {
                if i >= self.items.len() - 1 {
                    0
                } else {
                    i + 1
                }
            }
            None => 0,
        };
        self.list_state.select(Some(i));
    }

    fn previous(&mut self) {
        let i = match self.list_state.selected() {
            Some(i) => {
                if i == 0 {
                    self.items.len() - 1
                } else {
                    i - 1
                }
            }
            None => 0,
        };
        self.list_state.select(Some(i));
    }
}

fn ui(f: &mut Frame, app: &TuiApp) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Length(3), Constraint::Min(0)])
        .split(f.area());

    let title = Paragraph::new("Plum Package Manager (PPM) TUI")
        .style(Style::default().fg(Color::Green))
        .block(Block::default().borders(Borders::ALL));

    let items: Vec<ListItem> = app.items.iter().map(|s| ListItem::new(s.as_str())).collect();
    let list = List::new(items)
        .block(Block::default().title("Actions").borders(Borders::ALL))
        .highlight_style(Style::default().fg(Color::Yellow))
        .highlight_symbol(">> ");

    f.render_widget(title, chunks[0]);
    f.render_stateful_widget(list, chunks[1], &mut app.list_state.clone());
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    enable_raw_mode()?;
    let _ = execute!(io::stdout(), EnterAlternateScreen);
    let backend = CrosstermBackend::new(io::stdout());
    let mut terminal = Terminal::new(backend)?;

    let mut app = TuiApp::new();

    loop {
        terminal.draw(|f| ui(f, &app))?;

        if let Event::Key(key) = event::read()? {
            match key.code {
                KeyCode::Char('q') => break,
                KeyCode::Down => app.next(),
                KeyCode::Up => app.previous(),
                _ => {}
            }
        }
    }

    disable_raw_mode()?;
    let _ = execute!(io::stdout(), LeaveAlternateScreen);
    Ok(())
}