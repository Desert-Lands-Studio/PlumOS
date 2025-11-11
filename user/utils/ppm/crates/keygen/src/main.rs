use std::env;
use std::path::PathBuf;
use ed25519_dalek::SigningKey;
use getrandom;
use hex;

fn main() {
    let out_dir = env::args().nth(1).unwrap_or_else(|| ".".to_string());
    let out_path = PathBuf::from(out_dir);

    std::fs::create_dir_all(&out_path).expect("Failed to create output directory");

    let mut secret_key_bytes = [0u8; 32];
    getrandom::fill(&mut secret_key_bytes).expect("Failed to generate key");
    let signing_key = SigningKey::from_bytes(&secret_key_bytes);

    let priv_hex = hex::encode(signing_key.to_bytes());
    std::fs::write(out_path.join("signing-key.hex"), &priv_hex).unwrap();

    let pub_hex = hex::encode(signing_key.verifying_key().to_bytes());
    std::fs::write(out_path.join("repo_key.pubhex"), &pub_hex).unwrap();

    println!("✅ Keys generated in {}", out_path.display());
}