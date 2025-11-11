pub mod architecture;
pub mod channel;
pub mod config;
pub mod package;
pub mod security;
pub mod repository;
pub mod error;
pub mod formats;

pub use architecture::Architecture;
pub use channel::Channel;
pub use config::Config;
pub use package::{Package, PackageMetadata, PackageIndex};
pub use security::{verify_signature, compute_checksum, generate_keypair};
pub use error::{Result, PpmError};
pub use repository::{Repository, RepositoryManager};

pub use plum_formats::plam;

#[cfg(not(target_os = "none"))]
pub mod operations;
#[cfg(not(target_os = "none"))]
pub use operations::{
    load_config,
    install_package,
    remove_package,
    update_packages,
    search_packages,
    show_package_info,
    list_packages,
    check_updates,
    clean_cache,
};