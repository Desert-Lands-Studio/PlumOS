use std::fs;
use std::io;
use std::process::Command;
use ratatui::{prelude::*, widgets::*};
use crossterm::{
    event::{self, Event, KeyCode, KeyEventKind},
    terminal::{enable_raw_mode, disable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
    execute,
};
use serde::{Serialize, Deserialize};
use dialoguer::{Confirm, Input, Select};

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct PlumConfig {
    system: SystemConfig,
    drivers: DriversConfig,
    compatibility: CompatibilityConfig,
    build: BuildConfig,
    plam: PlamConfig,
}

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct DriversConfig {
    usb: bool,
    wifi: bool,
    gpu: bool,
    nvme: bool,
    audio: bool,
    bluetooth: bool,
}

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct CompatibilityConfig {
    posix: bool,
    win32: bool,
    darwin: bool,
    android: bool,
    linux: bool,
}

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct BuildConfig {
    mode: String,
    optimization: String,
    debug_info: bool,
    lto: bool,
}

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct PlamConfig {
    subsystem: String,
    flags: Vec<String>,
    version: String,
}

#[derive(Debug, Serialize, Deserialize, Default, Clone)]
struct SystemConfig {
    arch: String,
    page_size: String,
    smp_cores: usize,
    secure_boot: bool,
    memory_size: String,
    boot_delay: u32,
}

#[derive(PartialEq)]
enum InputMode {
    Normal,
    Editing,
}

fn main() -> io::Result<()> {
    let config_path = "plum.config";
    let mut cfg: PlumConfig = if let Ok(data) = fs::read_to_string(&config_path) {
        toml::from_str(&data).unwrap_or_else(|e| {
            eprintln!("Warning: Error parsing config: {}. Using defaults.", e);
            Default::default()
        })
    } else {
        println!("No existing config found. Creating default configuration.");
        PlumConfig {
            system: SystemConfig {
                arch: "prum64".into(),
                page_size: "16K".into(),
                smp_cores: 8,
                secure_boot: true,
                memory_size: "1G".into(),
                boot_delay: 3,
            },
            drivers: DriversConfig {
                usb: true,
                wifi: true,
                gpu: false,
                nvme: true,
                audio: false,
                bluetooth: false,
            },
            compatibility: CompatibilityConfig {
                posix: true,
                win32: true,
                darwin: true,
                android: false,
                linux: true,
            },
            build: BuildConfig {
                mode: "release".into(),
                optimization: "speed".into(),
                debug_info: false,
                lto: true,
            },
            plam: PlamConfig {
                subsystem: "native_kernel".into(),
                flags: vec!["pie".into(), "aslr".into()],
                version: "1.0.0".into(),
            },
        }
    };

    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    let mut selected_tab = 0;
    let mut selected_item = 0;
    let tabs = vec!["System", "Drivers", "Compatibility", "Build", "PLAM", "Actions"];
    let mut input_mode = InputMode::Normal;
    let mut input_buffer = String::new();
    let mut current_edit: Option<(usize, usize)> = None;
    let mut message: Option<String> = None;

    loop {
        terminal.draw(|f| ui(f, &cfg, selected_tab, selected_item, &tabs, &input_mode, &input_buffer, current_edit, &message))?;

        if let Some(msg) = message.take() {
            println!("{}", msg);
        }

        if event::poll(std::time::Duration::from_millis(100))? {
            if let Event::Key(key) = event::read()? {
                if key.kind == KeyEventKind::Press {
                    match input_mode {
                        InputMode::Normal => match key.code {
                            KeyCode::Char('q') => break,
                            KeyCode::Char('s') => {
                                message = Some(save_and_generate(&cfg, &config_path));
                            }
                            KeyCode::Char('b') => {
                                if build_system(&cfg).is_ok() {
                                    message = Some("Build completed successfully!".into());
                                } else {
                                    message = Some("Build failed!".into());
                                }
                            }
                            KeyCode::Left => {
                                selected_tab = selected_tab.saturating_sub(1);
                                selected_item = 0;
                            }
                            KeyCode::Right => {
                                selected_tab = (selected_tab + 1).min(tabs.len() - 1);
                                selected_item = 0;
                            }
                            KeyCode::Up => selected_item = selected_item.saturating_sub(1),
                            KeyCode::Down => {
                                selected_item = (selected_item + 1).min(get_max_items(selected_tab) - 1)
                            }
                            KeyCode::Enter => {
                                if selected_tab == 5 {
                                    // Actions tab
                                    handle_action(&mut cfg, selected_item, &mut message);
                                } else if selected_tab == 4 && selected_item == 1 {
                                    // PLAM flags editing
                                    input_mode = InputMode::Editing;
                                    input_buffer = cfg.plam.flags.join(", ");
                                    current_edit = Some((selected_tab, selected_item));
                                } else if can_edit_directly(selected_tab, selected_item) {
                                    input_mode = InputMode::Editing;
                                    input_buffer = get_current_value(&cfg, selected_tab, selected_item);
                                    current_edit = Some((selected_tab, selected_item));
                                } else {
                                    handle_selection(&mut cfg, selected_tab, selected_item);
                                }
                            }
                            KeyCode::Char(' ') => handle_toggle(&mut cfg, selected_tab, selected_item),
                            _ => {}
                        },
                        InputMode::Editing => match key.code {
                            KeyCode::Enter => {
                                if apply_edit(&mut cfg, &input_buffer, current_edit).is_ok() {
                                    message = Some("Value updated successfully".into());
                                } else {
                                    message = Some("Error: Invalid value".into());
                                }
                                input_mode = InputMode::Normal;
                                input_buffer.clear();
                                current_edit = None;
                            }
                            KeyCode::Esc => {
                                input_mode = InputMode::Normal;
                                input_buffer.clear();
                                current_edit = None;
                                message = Some("Edit cancelled".into());
                            }
                            KeyCode::Char(c) => {
                                input_buffer.push(c);
                            }
                            KeyCode::Backspace => {
                                input_buffer.pop();
                            }
                            _ => {}
                        },
                    }
                }
            }
        }
    }

    disable_raw_mode()?;
    execute!(io::stdout(), LeaveAlternateScreen)?;
    Ok(())
}

fn get_max_items(tab: usize) -> usize {
    match tab {
        0 => 6, // System: arch, page_size, smp_cores, secure_boot, memory_size, boot_delay
        1 => 6, // Drivers: usb, wifi, gpu, nvme, audio, bluetooth
        2 => 5, // Compatibility: posix, win32, darwin, android, linux
        3 => 4, // Build: mode, optimization, debug_info, lto
        4 => 3, // PLAM: subsystem, flags, version
        5 => 3, // Actions: reset, validate, build
        _ => 1,
    }
}

fn can_edit_directly(tab: usize, item: usize) -> bool {
    matches!((tab, item), 
        (0, 2) | // SMP Cores
        (0, 4) | // Memory size
        (0, 5) | // Boot delay
        (4, 1) | // PLAM flags
        (4, 2)   // PLAM version
    )
}

fn get_current_value(cfg: &PlumConfig, tab: usize, item: usize) -> String {
    match (tab, item) {
        (0, 2) => cfg.system.smp_cores.to_string(),
        (0, 4) => cfg.system.memory_size.clone(),
        (0, 5) => cfg.system.boot_delay.to_string(),
        (4, 1) => cfg.plam.flags.join(", "),
        (4, 2) => cfg.plam.version.clone(),
        _ => String::new(),
    }
}

fn apply_edit(cfg: &mut PlumConfig, input: &str, current_edit: Option<(usize, usize)>) -> Result<(), &'static str> {
    if let Some((tab, item)) = current_edit {
        match (tab, item) {
            (0, 2) => { // SMP Cores
                if let Ok(cores) = input.parse::<usize>() {
                    if cores > 0 && cores <= 256 {
                        cfg.system.smp_cores = cores;
                        Ok(())
                    } else {
                        Err("SMP cores must be between 1 and 256")
                    }
                } else {
                    Err("Invalid number format")
                }
            }
            (0, 4) => { // Memory size
                if input.ends_with(['K', 'M', 'G']) || input.parse::<u64>().is_ok() {
                    cfg.system.memory_size = input.to_string();
                    Ok(())
                } else {
                    Err("Memory size must be like '1G', '512M', or number")
                }
            }
            (0, 5) => { // Boot delay
                if let Ok(delay) = input.parse::<u32>() {
                    if delay <= 60 {
                        cfg.system.boot_delay = delay;
                        Ok(())
                    } else {
                        Err("Boot delay must be <= 60 seconds")
                    }
                } else {
                    Err("Invalid number format")
                }
            }
            (4, 1) => { // PLAM flags
                cfg.plam.flags = input.split(',')
                    .map(|s| s.trim().to_string())
                    .filter(|s| !s.is_empty())
                    .collect();
                Ok(())
            }
            (4, 2) => { // PLAM version
                if !input.is_empty() {
                    cfg.plam.version = input.to_string();
                    Ok(())
                } else {
                    Err("Version cannot be empty")
                }
            }
            _ => Ok(()),
        }
    } else {
        Ok(())
    }
}

fn handle_selection(cfg: &mut PlumConfig, tab: usize, item: usize) {
    match (tab, item) {
        (0, 0) => { // Architecture
            let choices = vec!["prum64", "aarch64", "x86_64", "riscv64"];
            let current_index = choices.iter().position(|&x| x == cfg.system.arch).unwrap_or(0);
            let next_index = (current_index + 1) % choices.len();
            cfg.system.arch = choices[next_index].to_string();
        }
        (0, 1) => { // Page size
            let choices = vec!["4K", "8K", "16K", "64K"];
            let current_index = choices.iter().position(|&x| x == cfg.system.page_size).unwrap_or(0);
            let next_index = (current_index + 1) % choices.len();
            cfg.system.page_size = choices[next_index].to_string();
        }
        (3, 0) => { // Build mode
            let choices = vec!["debug", "release", "profile"];
            let current_index = choices.iter().position(|&x| x == cfg.build.mode).unwrap_or(0);
            let next_index = (current_index + 1) % choices.len();
            cfg.build.mode = choices[next_index].to_string();
        }
        (3, 1) => { // Optimization
            let choices = vec!["none", "speed", "size", "balanced"];
            let current_index = choices.iter().position(|&x| x == cfg.build.optimization).unwrap_or(0);
            let next_index = (current_index + 1) % choices.len();
            cfg.build.optimization = choices[next_index].to_string();
        }
        (4, 0) => { // Subsystem
            let choices = vec!["native_kernel", "driver", "console_app", "gui_app", "wasm", "firmware"];
            let current_index = choices.iter().position(|&x| x == cfg.plam.subsystem).unwrap_or(0);
            let next_index = (current_index + 1) % choices.len();
            cfg.plam.subsystem = choices[next_index].to_string();
        }
        _ => {}
    }
}

fn handle_toggle(cfg: &mut PlumConfig, tab: usize, item: usize) {
    match (tab, item) {
        (0, 3) => cfg.system.secure_boot = !cfg.system.secure_boot,
        (1, 0) => cfg.drivers.usb = !cfg.drivers.usb,
        (1, 1) => cfg.drivers.wifi = !cfg.drivers.wifi,
        (1, 2) => cfg.drivers.gpu = !cfg.drivers.gpu,
        (1, 3) => cfg.drivers.nvme = !cfg.drivers.nvme,
        (1, 4) => cfg.drivers.audio = !cfg.drivers.audio,
        (1, 5) => cfg.drivers.bluetooth = !cfg.drivers.bluetooth,
        (2, 0) => cfg.compatibility.posix = !cfg.compatibility.posix,
        (2, 1) => cfg.compatibility.win32 = !cfg.compatibility.win32,
        (2, 2) => cfg.compatibility.darwin = !cfg.compatibility.darwin,
        (2, 3) => cfg.compatibility.android = !cfg.compatibility.android,
        (2, 4) => cfg.compatibility.linux = !cfg.compatibility.linux,
        (3, 2) => cfg.build.debug_info = !cfg.build.debug_info,
        (3, 3) => cfg.build.lto = !cfg.build.lto,
        _ => {}
    }
}

fn handle_action(cfg: &mut PlumConfig, item: usize, message: &mut Option<String>) {
    match item {
        0 => { // Reset to defaults
            *cfg = Default::default();
            *message = Some("Configuration reset to defaults".into());
        }
        1 => { // Validate configuration
            if validate_config(cfg) {
                *message = Some("✓ Configuration is valid".into());
            } else {
                *message = Some("⚠ Configuration has issues".into());
            }
        }
        2 => { // Build system
            if build_system(cfg).is_ok() {
                *message = Some("Build completed successfully!".into());
            } else {
                *message = Some("Build failed!".into());
            }
        }
        _ => {}
    }
}

fn validate_config(cfg: &PlumConfig) -> bool {
    let mut valid = true;
    
    if cfg.system.smp_cores == 0 {
        eprintln!("Error: SMP cores cannot be 0");
        valid = false;
    }
    
    if cfg.system.memory_size.is_empty() {
        eprintln!("Error: Memory size cannot be empty");
        valid = false;
    }
    
    if cfg.plam.version.is_empty() {
        eprintln!("Warning: PLAM version is empty");
    }
    
    valid
}

fn build_system(cfg: &PlumConfig) -> Result<(), Box<dyn std::error::Error>> {
    println!("Building system with configuration...");
    println!("Architecture: {}", cfg.system.arch);
    println!("Mode: {}", cfg.build.mode);
    
    // Здесь будет реальная логика сборки
    // Пока просто имитируем успешную сборку
    Ok(())
}

fn ui(f: &mut Frame, cfg: &PlumConfig, selected_tab: usize, selected_item: usize, tabs: &[&str], 
      input_mode: &InputMode, input_buffer: &str, current_edit: Option<(usize, usize)>, message: &Option<String>) {
    let size = f.area();
    
    // BSD-style blue background
    let background = Block::default().style(Style::default().bg(Color::Blue));
    f.render_widget(background, size);

    let main_area = centered_rect(85, 75, size);
    
    // Main window
    let mut main_block = Block::default()
        .title(" PlumOS Kernel Configuration ")
        .borders(Borders::ALL)
        .border_style(Style::default().fg(Color::White))
        .style(Style::default().bg(Color::Blue));
    
    // Добавляем сообщение в заголовок, если есть
    if let Some(msg) = message {
        main_block = main_block.title(format!(" PlumOS Kernel Configuration - {} ", msg));
    }
    
    f.render_widget(main_block, main_area);

    let inner_area = main_area.inner(Margin::new(1, 1));
    
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(3), // Tabs
            Constraint::Min(12),   // Content
            Constraint::Length(4), // Help
        ])
        .split(inner_area);

    // Tabs
    let tab_titles = tabs
        .iter()
        .map(|t| {
            let (first, rest) = t.split_at(1);
            Line::from(vec![
                Span::styled(first, Style::default().fg(Color::Yellow)),
                Span::styled(rest, Style::default().fg(Color::White)),
            ])
        })
        .collect::<Vec<Line>>();

    let tabs = Tabs::new(tab_titles)
        .block(Block::default().borders(Borders::BOTTOM).style(Style::default().bg(Color::Blue)))
        .select(selected_tab)
        .style(Style::default().fg(Color::White))
        .highlight_style(Style::default().fg(Color::Yellow).bg(Color::Blue))
        .divider("│");

    f.render_widget(tabs, chunks[0]);

    // Content
    let content_area = chunks[1];
    match selected_tab {
        0 => render_system_tab(f, cfg, selected_item, content_area, input_mode, input_buffer, current_edit),
        1 => render_drivers_tab(f, cfg, selected_item, content_area),
        2 => render_compatibility_tab(f, cfg, selected_item, content_area),
        3 => render_build_tab(f, cfg, selected_item, content_area),
        4 => render_plam_tab(f, cfg, selected_item, content_area, input_mode, input_buffer, current_edit),
        5 => render_actions_tab(f, selected_item, content_area),
        _ => {}
    }

    // Help section
    let help_text = match input_mode {
        InputMode::Normal => Line::from(vec![
            Span::styled("←/→", Style::default().fg(Color::Yellow)),
            Span::raw(" Tabs "),
            Span::styled("↑/↓", Style::default().fg(Color::Yellow)),
            Span::raw(" Navigate "),
            Span::styled("Enter", Style::default().fg(Color::Yellow)),
            Span::raw(" Edit/Select "),
            Span::styled("Space", Style::default().fg(Color::Yellow)),
            Span::raw(" Toggle "),
            Span::styled("S", Style::default().fg(Color::Yellow)),
            Span::raw(" Save "),
            Span::styled("B", Style::default().fg(Color::Yellow)),
            Span::raw(" Build "),
            Span::styled("Q", Style::default().fg(Color::Yellow)),
            Span::raw(" Quit"),
        ]),
        InputMode::Editing => Line::from(vec![
            Span::styled("Enter", Style::default().fg(Color::Yellow)),
            Span::raw(" Save "),
            Span::styled("Esc", Style::default().fg(Color::Yellow)),
            Span::raw(" Cancel "),
            Span::raw(" | Editing: "),
            Span::styled(input_buffer, Style::default().fg(Color::Green)),
        ]),
    };

    let help_block = Block::default()
        .borders(Borders::TOP)
        .border_style(Style::default().fg(Color::White))
        .style(Style::default().bg(Color::Blue));

    let help_paragraph = Paragraph::new(help_text)
        .block(help_block)
        .alignment(Alignment::Center);

    f.render_widget(help_paragraph, chunks[2]);
}

fn render_system_tab(f: &mut Frame, cfg: &PlumConfig, selected: usize, area: Rect, 
                    input_mode: &InputMode, input_buffer: &str, current_edit: Option<(usize, usize)>) {
    let items = vec![
        format!("Architecture:        {}", cfg.system.arch),
        format!("Page size:           {}", cfg.system.page_size),
        if current_edit == Some((0, 2)) && *input_mode == InputMode::Editing {
            format!("SMP Cores:           {}", input_buffer)
        } else {
            format!("SMP Cores:           {}", cfg.system.smp_cores)
        },
        format!("Secure Boot:         {}", if cfg.system.secure_boot { "[✓]" } else { "[ ]" }),
        if current_edit == Some((0, 4)) && *input_mode == InputMode::Editing {
            format!("Memory Size:         {}", input_buffer)
        } else {
            format!("Memory Size:         {}", cfg.system.memory_size)
        },
        if current_edit == Some((0, 5)) && *input_mode == InputMode::Editing {
            format!("Boot Delay:          {} seconds", input_buffer)
        } else {
            format!("Boot Delay:          {} seconds", cfg.system.boot_delay)
        },
    ];

    render_list(f, &items, selected, area, "System Configuration");
}

fn render_drivers_tab(f: &mut Frame, cfg: &PlumConfig, selected: usize, area: Rect) {
    let items = vec![
        format!("USB:                 {}", if cfg.drivers.usb { "[✓]" } else { "[ ]" }),
        format!("WiFi:                {}", if cfg.drivers.wifi { "[✓]" } else { "[ ]" }),
        format!("GPU:                 {}", if cfg.drivers.gpu { "[✓]" } else { "[ ]" }),
        format!("NVMe:                {}", if cfg.drivers.nvme { "[✓]" } else { "[ ]" }),
        format!("Audio:               {}", if cfg.drivers.audio { "[✓]" } else { "[ ]" }),
        format!("Bluetooth:           {}", if cfg.drivers.bluetooth { "[✓]" } else { "[ ]" }),
    ];

    render_list(f, &items, selected, area, "Driver Configuration");
}

fn render_compatibility_tab(f: &mut Frame, cfg: &PlumConfig, selected: usize, area: Rect) {
    let items = vec![
        format!("POSIX:               {}", if cfg.compatibility.posix { "[✓]" } else { "[ ]" }),
        format!("Win32:               {}", if cfg.compatibility.win32 { "[✓]" } else { "[ ]" }),
        format!("Darwin:              {}", if cfg.compatibility.darwin { "[✓]" } else { "[ ]" }),
        format!("Android:             {}", if cfg.compatibility.android { "[✓]" } else { "[ ]" }),
        format!("Linux:               {}", if cfg.compatibility.linux { "[✓]" } else { "[ ]" }),
    ];

    render_list(f, &items, selected, area, "Compatibility Configuration");
}

fn render_build_tab(f: &mut Frame, cfg: &PlumConfig, selected: usize, area: Rect) {
    let items = vec![
        format!("Build Mode:          {}", cfg.build.mode),
        format!("Optimization:        {}", cfg.build.optimization),
        format!("Debug Info:          {}", if cfg.build.debug_info { "[✓]" } else { "[ ]" }),
        format!("LTO:                 {}", if cfg.build.lto { "[✓]" } else { "[ ]" }),
    ];

    render_list(f, &items, selected, area, "Build Configuration");
}

fn render_plam_tab(f: &mut Frame, cfg: &PlumConfig, selected: usize, area: Rect, 
                  input_mode: &InputMode, input_buffer: &str, current_edit: Option<(usize, usize)>) {
    let items = vec![
        format!("Subsystem:           {}", cfg.plam.subsystem),
        if current_edit == Some((4, 1)) && *input_mode == InputMode::Editing {
            format!("Flags:               {}", input_buffer)
        } else {
            format!("Flags:               {}", cfg.plam.flags.join(", "))
        },
        if current_edit == Some((4, 2)) && *input_mode == InputMode::Editing {
            format!("Version:             {}", input_buffer)
        } else {
            format!("Version:             {}", cfg.plam.version)
        },
    ];

    render_list(f, &items, selected, area, "PLAM Configuration");
}

fn render_actions_tab(f: &mut Frame, selected: usize, area: Rect) {
    let items = vec![
        "Reset to Defaults".to_string(),
        "Validate Configuration".to_string(),
        "Build System".to_string(),
    ];

    render_list(f, &items, selected, area, "Actions");
}

fn render_list(f: &mut Frame, items: &[String], selected: usize, area: Rect, title: &str) {
    let block = Block::default()
        .title(title)
        .style(Style::default().bg(Color::Blue));

    let list_items: Vec<ListItem> = items
        .iter()
        .enumerate()
        .map(|(i, item)| {
            let content = if i == selected {
                Line::from(vec![
                    Span::styled("➤ ", Style::default().fg(Color::Yellow)),
                    Span::styled(item, Style::default().fg(Color::Yellow)),
                ])
            } else {
                Line::from(vec![Span::raw("  "), Span::raw(item)])
            };
            ListItem::new(content)
        })
        .collect();

    let list = List::new(list_items)
        .block(block)
        .style(Style::default().fg(Color::White).bg(Color::Blue));

    f.render_widget(list, area);
}

fn centered_rect(percent_x: u16, percent_y: u16, r: Rect) -> Rect {
    let popup_layout = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Percentage((100 - percent_y) / 2),
            Constraint::Percentage(percent_y),
            Constraint::Percentage((100 - percent_y) / 2),
        ])
        .split(r);

    Layout::default()
        .direction(Direction::Horizontal)
        .constraints([
            Constraint::Percentage((100 - percent_x) / 2),
            Constraint::Percentage(percent_x),
            Constraint::Percentage((100 - percent_x) / 2),
        ])
        .split(popup_layout[1])[1]
}

fn save_and_generate(cfg: &PlumConfig, path: &str) -> String {
    // Валидация конфигурации перед сохранением
    if !validate_config(cfg) {
        return "Error: Configuration validation failed".into();
    }

    // Сохранение TOML конфигурации
    match toml::to_string_pretty(cfg) {
        Ok(toml) => {
            if let Err(e) = fs::write(path, &toml) {
                return format!("Error saving config: {}", e);
            }
        }
        Err(e) => {
            return format!("Error serializing config: {}", e);
        }
    }

    let page_size_value = match cfg.system.page_size.as_str() {
        "4K" => 4096,
        "8K" => 8192,
        "16K" => 16384,
        "64K" => 65536,
        _ => 4096,
    };

    let memory_bytes = match cfg.system.memory_size.as_str() {
        s if s.ends_with('K') => s.trim_end_matches('K').parse::<u64>().unwrap_or(0) * 1024,
        s if s.ends_with('M') => s.trim_end_matches('M').parse::<u64>().unwrap_or(0) * 1024 * 1024,
        s if s.ends_with('G') => s.trim_end_matches('G').parse::<u64>().unwrap_or(0) * 1024 * 1024 * 1024,
        s => s.parse::<u64>().unwrap_or(512 * 1024 * 1024), // default 512MB
    };

    let content = format!(
        "// Auto-generated configuration file\n\
        // Generated from: {}\n\n\
        pub mod config {{\n\
        use core::time::Duration;\n\n\
        // System Configuration\n\
        pub const ARCH: &str = \"{}\";\n\
        pub const PAGE_SIZE: usize = {};\n\
        pub const SMP_CORES: usize = {};\n\
        pub const SECURE_BOOT: bool = {};\n\
        pub const MEMORY_SIZE: u64 = {};\n\
        pub const BOOT_DELAY: Duration = Duration::from_secs({});\n\n\
        // Driver Configuration\n\
        pub const DRIVER_USB: bool = {};\n\
        pub const DRIVER_WIFI: bool = {};\n\
        pub const DRIVER_GPU: bool = {};\n\
        pub const DRIVER_NVME: bool = {};\n\
        pub const DRIVER_AUDIO: bool = {};\n\
        pub const DRIVER_BLUETOOTH: bool = {};\n\n\
        // Compatibility Configuration\n\
        pub const ABI_POSIX: bool = {};\n\
        pub const ABI_WIN32: bool = {};\n\
        pub const ABI_DARWIN: bool = {};\n\
        pub const ABI_ANDROID: bool = {};\n\
        pub const ABI_LINUX: bool = {};\n\n\
        // Build Configuration\n\
        pub const BUILD_MODE: &str = \"{}\";\n\
        pub const BUILD_OPTIMIZATION: &str = \"{}\";\n\
        pub const BUILD_DEBUG_INFO: bool = {};\n\
        pub const BUILD_LTO: bool = {};\n\n\
        // PLAM Configuration\n\
        pub const PLAM_SUBSYSTEM: &str = \"{}\";\n\
        pub const PLAM_FLAGS: &[&str] = &{:?};\n\
        pub const PLAM_VERSION: &str = \"{}\";\n\
    }}\n",
        path,
        cfg.system.arch,
        page_size_value,
        cfg.system.smp_cores,
        cfg.system.secure_boot,
        memory_bytes,
        cfg.system.boot_delay,
        cfg.drivers.usb,
        cfg.drivers.wifi,
        cfg.drivers.gpu,
        cfg.drivers.nvme,
        cfg.drivers.audio,
        cfg.drivers.bluetooth,
        cfg.compatibility.posix,
        cfg.compatibility.win32,
        cfg.compatibility.darwin,
        cfg.compatibility.android,
        cfg.compatibility.linux,
        cfg.build.mode,
        cfg.build.optimization,
        cfg.build.debug_info,
        cfg.build.lto,
        cfg.plam.subsystem,
        cfg.plam.flags,
        cfg.plam.version
    );

    // Создание директорий если не существуют
    let _ = fs::create_dir_all("kernel/pmhk/src");
    
    match fs::write("kernel/pmhk/src/config.rs", &content) {
        Ok(_) => format!("✅ Configuration saved to {} and generated at kernel/pmhk/src/config.rs", path),
        Err(e) => format!("Error generating config.rs: {}", e),
    }
}