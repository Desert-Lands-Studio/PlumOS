use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Architecture {
    #[serde(rename = "x86_64")]
    X86_64,
    #[serde(rename = "aarch64")]
    AArch64,
    #[serde(rename = "riscv64")]
    RiscV64,
    #[serde(rename = "prum64")]
    Prum64,
}

impl Architecture {
    pub fn current() -> Self {
        match std::env::consts::ARCH {
            "x86_64" => Architecture::X86_64,
            "aarch64" => Architecture::AArch64,
            "riscv64" => Architecture::RiscV64,
            _ => Architecture::Prum64,
        }
    }

    pub fn as_str(&self) -> &'static str {
        match self {
            Architecture::X86_64 => "x86_64",
            Architecture::AArch64 => "aarch64",
            Architecture::RiscV64 => "riscv64",
            Architecture::Prum64 => "prum64",
        }
    }

    pub fn supported_architectures() -> Vec<Self> {
        vec![
            Architecture::X86_64,
            Architecture::AArch64,
            Architecture::RiscV64,
            Architecture::Prum64,
        ]
    }
}

impl std::fmt::Display for Architecture {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.as_str())
    }
}

use std::str::FromStr;

impl FromStr for Architecture {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "x86_64" => Ok(Architecture::X86_64),
            "aarch64" => Ok(Architecture::AArch64),
            "riscv64" => Ok(Architecture::RiscV64),
            "prum64" => Ok(Architecture::Prum64),
            _ => Err(format!("Unknown architecture: {}", s)),
        }
    }
}