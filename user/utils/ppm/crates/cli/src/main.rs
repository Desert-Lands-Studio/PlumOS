use clap::{Parser, Subcommand};
use ppm_core::{
    load_config,
    install_package, remove_package, update_packages, search_packages,
    show_package_info, list_packages, check_updates, clean_cache,
    Channel, Architecture, Config,
};
use std::str::FromStr;

#[derive(Parser)]
#[command(name = "ppm", version, about)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Install {
        package: String,
        #[arg(short, long)]
        version: Option<String>,
        #[arg(short, long)]
        channel: Option<String>,
        #[arg(short, long)]
        arch: Option<String>,
        #[arg(long, default_value_t = true)]
        deps: bool,
        #[arg(long, default_value_t = false)]
        sandbox: bool,
        #[arg(long, default_value_t = false)]
        force: bool,
    },
    Remove {
        package: String,
        #[arg(long, default_value_t = false)]
        force: bool,
    },
    Update {
        #[arg(short, long)]
        package: Option<String>,
        #[arg(short, long)]
        channel: Option<String>,
    },
    Search {
        query: String,
        #[arg(short, long)]
        channel: Option<String>,
    },
    Info { package: String },
    List {
        #[arg(short, long)]
        channel: Option<String>,
    },
    Channel {
        #[command(subcommand)]
        action: ChannelAction,
    },
    Check {
        #[arg(short, long)]
        channel: Option<String>,
    },
    Clean {
        #[arg(long, default_value_t = false)]
        all: bool,
    },
    Tui,
}

#[derive(Subcommand)]
enum ChannelAction {
    Set { name: String },
    List,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let cli = Cli::parse();
    let config = load_config().await?;

    match cli.command {
        Commands::Install {
            package,
            version,
            channel,
            arch,
            deps,
            sandbox,
            force,
        } => {
            let parsed_channel = channel
                .as_deref()
                .map(Channel::from_str)
                .transpose()?
                .or(Some(config.channel));
            let parsed_arch = arch
                .as_deref()
                .map(Architecture::from_str)
                .transpose()?
                .or(Some(config.architecture));

            install_package(
                &package,
                version.as_deref(),
                parsed_channel,
                parsed_arch,
                deps,
                sandbox,
                force,
                &config,
            ).await?;
        }
        Commands::Remove { package, force } => {
            remove_package(&package, force, &config).await?;
        }
        Commands::Update { package, channel } => {
            let parsed_channel = channel
                .as_deref()
                .map(Channel::from_str)
                .transpose()?
                .or(Some(config.channel));
            update_packages(package.as_deref(), parsed_channel, &config).await?;
        }
        Commands::Search { query, channel } => {
            let parsed_channel = channel
                .as_deref()
                .map(Channel::from_str)
                .transpose()?
                .or(Some(config.channel));
            search_packages(&query, parsed_channel, &config).await?;
        }
        Commands::Info { package } => {
            show_package_info(&package, &config).await?;
        }
        Commands::List { channel } => {
            let parsed_channel = channel
                .as_deref()
                .map(Channel::from_str)
                .transpose()?
                .or(Some(config.channel));
            list_packages(parsed_channel, &config).await?;
        }
        Commands::Channel { action } => {
            handle_channel_action(action, &config).await?;
        }
        Commands::Check { channel } => {
            let parsed_channel = channel
                .as_deref()
                .map(Channel::from_str)
                .transpose()?
                .or(Some(config.channel));
            check_updates(parsed_channel, &config).await?;
        }
        Commands::Clean { all } => {
            clean_cache(all, &config).await?;
        }
        Commands::Tui => {
            start_tui(&config).await?;
        }
    }

    Ok(())
}

async fn handle_channel_action(action: ChannelAction, config: &Config) -> Result<(), Box<dyn std::error::Error>> {
    match action {
        ChannelAction::Set { name } => {
            let new_channel = Channel::from_str(&name)?;
            println!("🎛️ Switching to channel: {} → {}", config.channel, new_channel);
        }
        ChannelAction::List => {
            println!("Available channels:");
            for ch in Channel::all_channels() {
                println!(" {} - {}", ch.emoji(), ch.display_name());
            }
        }
    }
    Ok(())
}

async fn start_tui(_config: &Config) -> Result<(), Box<dyn std::error::Error>> {
    eprintln!("TUI mode not implemented yet");
    Ok(())
}