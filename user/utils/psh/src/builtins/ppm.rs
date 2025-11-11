use crate::terminal::{Terminal, Color, Style};
use ppm_core::{
    load_config,
    install_package, remove_package, update_packages,
    search_packages, list_packages, show_package_info,
    check_updates, clean_cache,
    Channel, Architecture, Config,
};
use anyhow::Result;

pub async fn handle_ppm_command(args: &[&str]) -> Result<()> {
    let config = load_config().await.unwrap_or_default();
    let term = Terminal::new();

    if args.is_empty() {
        show_help(&term);
        return Ok(());
    }

    let cmd = args[0];
    let rest = &args[1..];

    match cmd {
        "install" | "i" => {
            if rest.is_empty() {
                term.println("Usage: ppm install <package>", Color::Red, Style::Normal);
                return Ok(());
            }
            install_package(
                rest[0],
                None,
                None,
                None,
                false,
                false,
                false,
                &config,
            ).await?;
        }
        "remove" | "r" => {
            if rest.is_empty() {
                term.println("Usage: ppm remove <package>", Color::Red, Style::Normal);
                return Ok(());
            }
            remove_package(rest[0], false, &config).await?;
        }
        "update" | "u" => {
            let package = rest.get(0).cloned();
            update_packages(package, None, &config).await?;
        }
        "search" | "s" => {
            if let Some(q) = rest.get(0) {
                search_packages(q, None, &config).await?;
            }
        }
        "list" | "l" => {
            list_packages(None, &config).await?;
        }
        "info" => {
            if let Some(pkg) = rest.get(0) {
                show_package_info(pkg, &config).await?; 
            }
        }
        "channel" | "ch" => {
            handle_channel(rest, &term).await?;
        }
        "check" | "k" => {
            check_updates(None, &config).await?;
        }
        "clean" | "c" => {
            clean_cache(false, &config).await?;
        }
        _ => {
            term.println(&format!("Unknown PPM command: {}", cmd), Color::Red, Style::Normal);
            show_help(&term);
        }
    }

    Ok(())
}

async fn handle_channel(args: &[&str], term: &Terminal) -> Result<()> {
    match args.get(0).map(|s| *s) {
        Some("set") | Some("s") => {
            let channel = args.get(1).ok_or_else(|| anyhow::anyhow!("Usage: ppm channel set <name>"))?;
            let valid = ["stable", "testing", "unstable", "dev"];
            if !valid.contains(channel) {
                term.println(
                    &format!("Invalid channel: {}. Use: stable, testing, unstable, dev", channel),
                    Color::Red,
                    Style::Normal,
                );
                return Ok(());
            }
            term.println(&format!("✅ Channel set to: {}", channel), Color::Green, Style::Bold);
        }
        Some("list") | Some("l") => {
            term.println("Available channels:", Color::Cyan, Style::Bold);
            for ch in Channel::all_channels() {
                term.println(&format!("  {} - {}", ch.emoji(), ch.display_name()), Color::White, Style::Normal);
            }
        }
        _ => {
            term.println("Usage: ppm channel [set <name> | list]", Color::Yellow, Style::Normal);
        }
    }
    Ok(())
}

fn show_help(term: &Terminal) {
    term.println("🍑 PPM — Plum Package Manager", Color::Yellow, Style::Bold);
    term.println("Usage: ppm <command> [args]", Color::White, Style::Normal);
    term.println("\nCommands:", Color::Cyan, Style::Bold);
    println!("  install <pkg>    Install a package");
    println!("  remove <pkg>     Remove a package");
    println!("  update [pkg]     Update packages");
    println!("  search <query>   Search packages");
    println!("  list             List installed packages");
    println!("  info <pkg>       Show package info");
    println!("  channel set <ch> Change channel");
    println!("  channel list     List channels");
    println!("  check            Check for updates");
    println!("  clean            Clean cache");
}