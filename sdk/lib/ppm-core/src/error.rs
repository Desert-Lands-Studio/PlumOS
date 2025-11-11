use thiserror::Error;

#[derive(Error, Debug)]
pub enum PpmError {
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),
    
    #[error("Serialization error: {0}")]
    Serialization(String), // Changed from serde_json::Error
    
    #[error("Signature verification failed: {0}")]
    Signature(#[from] ed25519_dalek::SignatureError),
    
    #[error("Invalid package format: {0}")]
    InvalidPackage(String),
    
    #[error("Package not found: {0}")]
    PackageNotFound(String),
    
    #[error("Dependency resolution failed: {0}")]
    DependencyResolution(String),
    
    #[error("Security violation: {0}")]
    SecurityViolation(String),
}

pub type Result<T> = std::result::Result<T, PpmError>;