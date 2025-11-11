pub mod hash;
pub mod signature;
pub mod validator;

use crate::formats::plam::PlamHeader;

pub struct SecurityContext {
    pub secure_boot_enabled: bool,
    pub allowed_keys: Vec<[u8; 32]>,
}

impl SecurityContext {
    pub fn verify_plam(&self, header: &PlamHeader, data: &[u8]) -> Result<(), SecurityError> {
        
        if header.magic != 0x504C414D {
            return Err(SecurityError::InvalidFormat);
        }
        
        
        if !hash::verify_crc32(header, data) {
            return Err(SecurityError::ChecksumMismatch);
        }
        
        
        if self.secure_boot_enabled {
            signature::verify_signature(header, data, &self.allowed_keys)?;
        }
        
        
        validator::validate_compatibility(header)?;
        
        Ok(())
    }
}

#[derive(Debug)]
pub enum SecurityError {
    InvalidFormat,
    ChecksumMismatch,
    InvalidSignature,
    IncompatibleVersion,
    UnsupportedArchitecture,
}