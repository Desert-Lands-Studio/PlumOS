use ed25519_dalek::{Signature, VerifyingKey, SigningKey, SignatureError, Verifier};
use sha2::{Sha256, Digest};

#[cfg(not(target_os = "none"))]
use rand::RngCore;

pub fn verify_signature(data: &[u8], signature: &[u8], public_key: &[u8]) -> Result<(), SignatureError> {
    // Convert public_key to [u8; 32]
    let public_key_array: [u8; 32] = public_key
        .try_into()
        .map_err(|_| SignatureError::new())?;

    // Convert signature to [u8; 64]
    let signature_array: [u8; 64] = signature
        .try_into()
        .map_err(|_| SignatureError::new())?;

    // Create verifying key from raw bytes
    let verifying_key = VerifyingKey::from_bytes(&public_key_array)?;

    // Create signature object
    let signature_obj = Signature::from_bytes(&signature_array);

    // Verify signature using the `Verifier` trait (now in scope)
    verifying_key.verify(data, &signature_obj)
}

pub fn compute_checksum(data: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(data);
    format!("{:x}", hasher.finalize())
}

pub fn generate_keypair() -> (SigningKey, VerifyingKey) {
    let mut secret_key_bytes = [0u8; 32];

    #[cfg(not(target_os = "none"))]
    {
        let mut rng = rand::thread_rng();
        rng.fill_bytes(&mut secret_key_bytes);
    }

    #[cfg(target_os = "none")]
    {
        // Fallback for no_std environments (non-cryptographic!)
        for i in 0..32 {
            secret_key_bytes[i] = (i * 7) as u8;
        }
    }

    let signing_key = SigningKey::from_bytes(&secret_key_bytes);
    let verifying_key = signing_key.verifying_key();
    (signing_key, verifying_key)
}