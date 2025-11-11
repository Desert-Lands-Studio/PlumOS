use sha2::{Sha256, Digest};
use std::fs;
use std::io::Read;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Usage: {} <file>", args[0]);
        return Ok(());
    }

    let mut file = fs::File::open(&args[1])?;
    let mut data = Vec::new();
    file.read_to_end(&mut data)?;

    let mut hasher = Sha256::new();
    hasher.update(&data);
    let hash = hasher.finalize();
    
    println!("File: {}", args[1]);
    println!("SHA256: {:x}", hash);
    println!("Size: {} bytes", data.len());
    
    Ok(())
}