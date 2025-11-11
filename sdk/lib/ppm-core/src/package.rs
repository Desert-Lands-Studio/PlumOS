use serde::{Deserialize, Serialize};
use crate::{Architecture, Channel};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Package {
    pub name: String,
    pub version: String,
    pub description: Option<String>,
    pub author: Option<String>,
    pub license: Option<String>,
    pub dependencies: Vec<String>,
    pub architecture: Architecture,
    pub channel: Channel,
    pub file: String,
    pub checksum: String,
    pub signature: Option<String>,
    pub size: u64,
    pub install_size: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageMetadata {
    pub name: String,
    pub version: String,
    pub description: Option<String>,
    pub author: Option<String>,
    pub license: Option<String>,
    pub dependencies: Vec<String>,
    pub architectures: Vec<Architecture>,
    pub channels: Vec<Channel>,
    pub build_script: Option<String>,
    pub install_script: Option<String>,
    pub sandbox_config: Option<SandboxConfig>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SandboxConfig {
    pub allowed_paths: Vec<String>,
    pub network_access: bool,
    pub system_calls: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PackageIndex {
    pub packages: Vec<Package>,
    pub generated: String,
    pub channel: Channel,
}