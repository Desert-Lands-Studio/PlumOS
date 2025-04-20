// src/config/parser.rs
pub struct Config {
    pub language: String,
    pub theme: String,
    pub entries: Vec<BootEntry>,
    pub timeout: Option<u32>,
    pub default_index: usize,
}
pub enum BootEntry {
    Kernel { name: String, path: String },
    Chain { name: String, os: boot::chainload::OS },
}

pub fn load_config() -> Result<Config, ()> {
    // Найти файл конфигурации:
    let config_data = if let Ok(data) = fs::fat::FileSystem::new(0 /* ESP lba */)?.read_file("boot.conf") {
        data
    } else {
        return Err(());
    };
    // Разобрать строки:
    let text = core::str::from_utf8(&config_data).map_err(|_| ())?;
    let mut config = Config { language: "en".to_string(), theme: "default".to_string(),
                               entries: Vec::new(), timeout: None, default_index: 0 };
    for line in text.lines() {
        let line = line.trim();
        if line.starts_with('#') || line.is_empty() {
            continue;
        }
        if let Some(eq_idx) = line.find('=') {
            let key = line[..eq_idx].trim();
            let val = line[eq_idx+1..].trim().trim_matches('"');
            match key {
                "language" => config.language = val.to_string(),
                "theme" => config.theme = val.to_string(),
                "timeout" => config.timeout = val.parse().ok(),
                "default_entry" => config.default_index = val.parse().unwrap_or(0),
                k if k.starts_with("entry") => {
                    // example k = "entry0_name" or "entry0_path" or "entry1_chain"
                    // parse index and attribute
                    // for brevity, assume format exactly as example above
                    let parts: Vec<_> = k.split('_').collect();
                    if parts.len() == 2 {
                        let idx: usize = parts[0][5..].parse().unwrap_or(0);
                        let attr = parts[1];
                        if idx >= config.entries.len() {
                            config.entries.resize_with(idx+1, || BootEntry::Kernel { name: "".into(), path: "".into() });
                        }
                        match attr {
                            "name" => {
                                match &mut config.entries[idx] {
                                    BootEntry::Kernel{name, ..} => *name = val.to_string(),
                                    BootEntry::Chain{name, ..} => *name = val.to_string(),
                                }
                            }
                            "path" => {
                                config.entries[idx] = BootEntry::Kernel { name: config.entries[idx].name().to_string(), path: val.to_string() };
                            }
                            "chain" => {
                                // val might be "Windows" or "Linux"
                                let os = match val.to_lowercase().as_str() {
                                    "windows" => boot::chainload::OS::Windows,
                                    "linux" => boot::chainload::OS::Linux,
                                    other => boot::chainload::OS::Other(other.to_string()),
                                };
                                config.entries[idx] = BootEntry::Chain { name: config.entries[idx].name().to_string(), os };
                            }
                            _ => {}
                        }
                    }
                }
                _ => {}
            }
        }
    }
    Ok(config)
}
