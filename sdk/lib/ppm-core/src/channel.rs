use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Channel {
    #[serde(rename = "stable")]
    Stable,
    #[serde(rename = "testing")]
    Testing,
    #[serde(rename = "unstable")]
    Unstable,
    #[serde(rename = "dev")]
    Dev,
}

impl Channel {
    pub fn name(&self) -> &'static str {
        match self {
            Channel::Stable => "stable",
            Channel::Testing => "testing",
            Channel::Unstable => "unstable",
            Channel::Dev => "dev",
        }
    }

    pub fn emoji(&self) -> &'static str {
        match self {
            Channel::Stable => "🍑",
            Channel::Testing => "🌸",
            Channel::Unstable => "🌾",
            Channel::Dev => "🧪",
        }
    }

    pub fn display_name(&self) -> &'static str {
        match self {
            Channel::Stable => "Plum (Stable)",
            Channel::Testing => "Blossom (Testing)",
            Channel::Unstable => "Seed (Unstable)",
            Channel::Dev => "Development",
        }
    }

    pub fn all_channels() -> Vec<Self> {
        vec![Channel::Stable, Channel::Testing, Channel::Unstable, Channel::Dev]
    }
}

impl std::fmt::Display for Channel {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{} {}", self.emoji(), self.display_name())
    }
}

use std::str::FromStr;

impl FromStr for Channel {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "stable" => Ok(Channel::Stable),
            "testing" => Ok(Channel::Testing),
            "unstable" => Ok(Channel::Unstable),
            "dev" => Ok(Channel::Dev),
            _ => Err(format!("Unknown channel: {}", s)),
        }
    }
}