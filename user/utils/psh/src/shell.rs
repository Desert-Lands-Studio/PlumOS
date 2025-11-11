use crate::{
    builtins::{find_builtin, handle_ppm_command},
    terminal::{Terminal, Color, Style},
};
use anyhow::Result;
use std::io::{self, Write};

pub struct Shell {
    terminal: Terminal,
    current_channel: String,
}

impl Shell {
    pub fn new() -> Self {
        Self {
            terminal: Terminal::new(),
            current_channel: "stable".to_string(),
        }
    }

    pub async fn run(&mut self) -> Result<()> {
        self.show_welcome();

        loop {
            self.display_prompt();
            if let Some(input) = self.read_input() {
                if input.trim().is_empty() {
                    continue;
                }
                if !self.handle_command(&input).await? {
                    break;
                }
            }
        }
        Ok(())
    }

    fn show_welcome(&self) {
        self.terminal.println(
            &format!("🍑 Welcome to Plum Shell v{}", env!("CARGO_PKG_VERSION")),
            Color::Yellow,
            Style::Bold,
        );
        self.terminal.println(
            "Type 'help' for commands or 'ppm' for package management",
            Color::Cyan,
            Style::Normal,
        );
        println!();
    }

    fn display_prompt(&self) {
        let dir = std::env::current_dir()
            .ok()
            .and_then(|p| p.file_name().map(|n| n.to_string_lossy().to_string()))
            .unwrap_or_else(|| "?".to_string());

        let color = match self.current_channel.as_str() {
            "stable" => Color::Green,
            "testing" => Color::Yellow,
            "unstable" => Color::Red,
            _ => Color::Blue,
        };

        self.terminal.print(&format!("[{}]", self.current_channel), color, Style::Bold);
        self.terminal.print(&format!(" {}> ", dir), Color::Magenta, Style::Bold);
        io::stdout().flush().unwrap();
    }

    fn read_input(&self) -> Option<String> {
        let mut input = String::new();
        io::stdin().read_line(&mut input).ok()?;
        Some(input.trim().to_string())
    }

    async fn handle_command(&mut self, input: &str) -> Result<bool> {
        let parts: Vec<&str> = input.split_whitespace().collect();
        let cmd = parts[0];
        let args = &parts[1..];

        match cmd {
            "exit" | "logout" => {
                self.terminal.println("👋 Goodbye!", Color::Green, Style::Normal);
                return Ok(false);
            }
            "ppm" => {
                handle_ppm_command(args).await?;
                return Ok(true);
            }
            _ => {
                if let Some(builtin) = find_builtin(cmd) {
                    builtin(args)?;
                    return Ok(true);
                }

                // Внешняя команда (host only)
                #[cfg(not(target_os = "none"))]
                {
                    use std::process::Command;
                    if let Err(_) = Command::new(cmd).args(args).status() {
                        self.terminal.println(&format!("Command not found: {}", cmd), Color::Red, Style::Normal);
                    }
                }
            }
        }
        Ok(true)
    }
}