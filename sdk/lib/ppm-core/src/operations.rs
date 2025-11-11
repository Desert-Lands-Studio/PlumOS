use crate::{
    Config, Package, Channel, Architecture, RepositoryManager, Repository,
    Result, PpmError, compute_checksum,
};
use std::path::Path;
use tokio::fs;

const DEFAULT_REPO_URL: &str = "https://repo.plumos.dev";

pub async fn load_config() -> Result<Config> {
    let config_path = dirs::config_dir()
        .unwrap_or_else(|| Path::new("/etc").to_path_buf())
        .join("ppm/config.toml");

    if config_path.exists() {
        let contents = fs::read_to_string(&config_path).await?;
        let config: Config = toml::from_str(&contents)
            .map_err(|e| PpmError::Serialization(e.to_string()))?;
        Ok(config)
    } else {
        let config = Config::default();
        save_config(&config).await?;
        Ok(config)
    }
}

pub async fn save_config(config: &Config) -> Result<()> {
    let config_dir = dirs::config_dir()
        .unwrap_or_else(|| Path::new("/etc").to_path_buf())
        .join("ppm");
    fs::create_dir_all(&config_dir).await?;
    let config_path = config_dir.join("config.toml");
    let contents = toml::to_string_pretty(config)
        .map_err(|e| PpmError::Serialization(e.to_string()))?;
    fs::write(&config_path, contents).await?;
    Ok(())
}

async fn get_repo_manager(config: &Config) -> Result<RepositoryManager> {
    let mut manager = RepositoryManager::new();
    let architectures = vec![config.architecture];
    let repo = Repository::new(
        DEFAULT_REPO_URL.to_string(),
        "main".to_string(),
        config.channel,
        architectures,
    );
    manager.add_repository(repo);
    // Добавьте другие репозитории из конфига, если реализуете их позже
    Ok(manager)
}

pub async fn install_package(
    package_name: &str,
    version: Option<&str>,
    channel: Option<Channel>,
    arch: Option<Architecture>,
    _deps: bool,
    _sandbox: bool,
    _force: bool,
    config: &Config,
) -> Result<()> {
    let ch = channel.unwrap_or(config.channel);
    let arch = arch.unwrap_or(config.architecture);
    let manager = get_repo_manager(config).await?;
    if let Some(pkg) = manager.find_package_across_repos(package_name, version)? {
        if pkg.channel == ch && pkg.architecture == arch {
            println!("📥 Installing {}-{} ({})", pkg.name, pkg.version, pkg.file);
            // Здесь будет настоящая установка: скачивание, проверка, распаковка
            Ok(())
        } else {
            Err(PpmError::PackageNotFound(format!(
                "No {} package for {} in {} channel", package_name, arch.as_str(), ch.name()
            )))
        }
    } else {
        Err(PpmError::PackageNotFound(package_name.to_string()))
    }
}

pub async fn remove_package(package_name: &str, _force: bool, _config: &Config) -> Result<()> {
    println!("🗑️ Removing package: {}", package_name);
    // Реальная логика: удаление файлов, запуск post-remove скрипта и т.д.
    Ok(())
}

pub async fn update_packages(
    package_name: Option<&str>,
    channel: Option<Channel>,
    config: &Config,
) -> Result<()> {
    let ch = channel.unwrap_or(config.channel);
    println!("🔄 Updating packages in {} channel...", ch.name());
    if let Some(name) = package_name {
        println!(" Target: {}", name);
    }
    // Реализация: сравнение версий, обновление
    Ok(())
}

pub async fn search_packages(query: &str, channel: Option<Channel>, config: &Config) -> Result<()> {
    let ch = channel.unwrap_or(config.channel);
    println!("🔍 Searching for '{}' in {} channel...", query, ch.name());
    // Реализация: поиск по индексу
    Ok(())
}

pub async fn show_package_info(package_name: &str, config: &Config) -> Result<()> {
    println!("📄 Showing info for package: {}", package_name);
    // Получить метаданные из индекса
    Ok(())
}

pub async fn list_packages(channel: Option<Channel>, config: &Config) -> Result<()> {
    let ch = channel.unwrap_or(config.channel);
    println!("📋 Listing packages in {} channel...", ch.name());
    // Список установленных пакетов (из /var/lib/ppm/db или подобного)
    Ok(())
}

pub async fn check_updates(channel: Option<Channel>, config: &Config) -> Result<()> {
    let ch = channel.unwrap_or(config.channel);
    println!("✅ Checking for updates in {} channel...", ch.name());
    // Сравнить локальные версии с репозиторием
    Ok(())
}

pub async fn clean_cache(all: bool, config: &Config) -> Result<()> {
    let cache_dir = &config.cache_dir;
    if all {
        println!("🧹 Cleaning entire cache at {}", cache_dir);
        // Удалить всё в кэше
    } else {
        println!("🧹 Cleaning old cache entries...");
        // Удалить только устаревшие
    }
    Ok(())
}