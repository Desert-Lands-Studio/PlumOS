mod ppm;
#[cfg(not(target_os = "none"))]
mod system;

pub use ppm::handle_ppm_command;

use crate::terminal::{Terminal, Color, Style};

pub type BuiltinFn = fn(&[&str]) -> anyhow::Result<()>;

pub struct Builtin {
    pub name: &'static str,
    pub func: BuiltinFn,
    pub help: &'static str,
}

pub const BUILTINS: &[Builtin] = &[
    Builtin {
        name: "exit",
        func: exit_shell,
        help: "Exit the shell",
    },
    Builtin {
        name: "clear",
        func: clear_screen,
        help: "Clear the terminal screen",
    },
    Builtin {
        name: "help",
        func: show_help,
        help: "Show this help message",
    },
    #[cfg(not(target_os = "none"))]
    Builtin {
        name: "ls",
        func: system::ls,
        help: "List directory contents",
    },
    #[cfg(not(target_os = "none"))]
    Builtin {
        name: "cd",
        func: system::cd,
        help: "Change current directory",
    },
    #[cfg(not(target_os = "none"))]
    Builtin {
        name: "pwd",
        func: system::pwd,
        help: "Print working directory",
    },
    #[cfg(not(target_os = "none"))]
    Builtin {
        name: "cat",
        func: system::cat,
        help: "Display file contents",
    },
    #[cfg(not(target_os = "none"))]
    Builtin {
        name: "echo",
        func: system::echo,
        help: "Print arguments to stdout",
    },
];

pub fn find_builtin(name: &str) -> Option<BuiltinFn> {
    BUILTINS.iter().find(|b| b.name == name).map(|b| b.func)
}

fn exit_shell(_args: &[&str]) -> anyhow::Result<()> {
    std::process::exit(0);
}

fn clear_screen(_args: &[&str]) -> anyhow::Result<()> {
    if cfg!(not(target_os = "none")) {
        print!("\x1B[2J\x1B[H");
    }
    Ok(())
}

fn show_help(_args: &[&str]) -> anyhow::Result<()> {
    let term = Terminal::new();
    term.println("🍑 Plum Shell — Built-in Commands", Color::Yellow, Style::Bold);
    term.println("==================================", Color::Cyan, Style::Normal);
    for builtin in BUILTINS {
        term.println(&format!("{:<10} - {}", builtin.name, builtin.help), Color::White, Style::Normal);
    }
    term.println("\nAlso supports: `ppm <cmd>` (built-in package manager)", Color::Green, Style::Normal);
    Ok(())
}