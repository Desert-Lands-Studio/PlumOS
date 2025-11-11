use serde::{Deserialize, Serialize};
use crate::{Package, PackageMetadata, Architecture, Channel};

/// Plum Package Manager формат пакета (.plpm)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlpmPackage {
    pub header: PlpmHeader,
    pub metadata: PackageMetadata,
    pub files: Vec<PlpmFile>,
    pub scripts: Option<PlpmScripts>,
    pub signature: Option<String>, // Changed from Vec<u8> to String
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlpmHeader {
    pub magic: [u8; 4],  // "PLPM"
    pub version: u16,
    pub architecture: Architecture,
    pub channel: Channel,
    pub compressed: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlpmFile {
    pub path: String,
    pub data: Vec<u8>,
    pub permissions: u32,
    pub checksum: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PlpmScripts {
    pub pre_install: Option<String>,
    pub post_install: Option<String>,
    pub pre_remove: Option<String>,
    pub post_remove: Option<String>,
}

impl PlpmPackage {
    pub fn new(package: Package, files: Vec<PlpmFile>) -> Self {
        Self {
            header: PlpmHeader {
                magic: *b"PLPM",
                version: 1,
                architecture: package.architecture,
                channel: package.channel,
                compressed: true,
            },
            metadata: PackageMetadata {
                name: package.name,
                version: package.version,
                description: package.description,
                author: package.author,
                license: package.license,
                dependencies: package.dependencies,
                architectures: vec![package.architecture],
                channels: vec![package.channel],
                build_script: None,
                install_script: None,
                sandbox_config: None,
            },
            files,
            scripts: None,
            signature: package.signature,
        }
    }
}