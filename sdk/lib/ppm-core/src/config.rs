use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub repository_url: String,
    pub cache_dir: String,
    pub keyring_dir: String,
    pub architecture: crate::Architecture,
    pub channel: crate::Channel,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            repository_url: "https://repo.plumos.dev".to_string(),
            cache_dir: "/var/cache/ppm".to_string(),
            keyring_dir: "/etc/ppm/keys".to_string(),
            architecture: crate::Architecture::current(),
            channel: crate::Channel::Stable,
        }
    }
}