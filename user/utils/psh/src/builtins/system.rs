use crate::terminal::{Terminal, Color, Style};
use anyhow::Result;
use std::fs;
use std::path::Path;

pub fn ls(args: &[&str]) -> Result<()> {
    let path = if !args.is_empty() { Path::new(args[0]) } else { Path::new(".") };
    match fs::read_dir(path) {
        Ok(entries) => {
            for entry in entries {
                if let Ok(entry) = entry {
                    print!("{} ", entry.file_name().to_string_lossy());
                }
            }
            println!();
        }
        Err(e) => {
            let term = Terminal::new();
            term.println(&format!("ls: cannot access '{}': {}", path.display(), e), Color::Red, Style::Normal);
        }
    }
    Ok(())
}

pub fn cd(args: &[&str]) -> Result<()> {
    if args.is_empty() {
        eprintln!("cd: missing argument");
        return Ok(());
    }
    let path = Path::new(args[0]);
    if let Err(e) = std::env::set_current_dir(path) {
        let term = Terminal::new();
        term.println(&format!("cd: {}: {}", path.display(), e), Color::Red, Style::Normal);
    }
    Ok(())
}

pub fn pwd(_args: &[&str]) -> Result<()> {
    if let Ok(cwd) = std::env::current_dir() {
        println!("{}", cwd.display());
    }
    Ok(())
}

pub fn cat(args: &[&str]) -> Result<()> {
    if args.is_empty() {
        eprintln!("cat: missing file operand");
        return Ok(());
    }
    for file in args {
        match fs::read_to_string(file) {
            Ok(contents) => print!("{}", contents),
            Err(e) => {
                let term = Terminal::new();
                term.println(&format!("cat: {}: {}", file, e), Color::Red, Style::Normal);
            }
        }
    }
    Ok(())
}

pub fn echo(args: &[&str]) -> Result<()> {
    println!("{}", args.join(" "));
    Ok(())
}