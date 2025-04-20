// src/fs/gpt_mbr.rs
use gpt::GptConfig;
use mbrman::MBR;

pub fn parse_gpt(disk: &mut std::fs::File) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    let gpt = GptConfig::new().read_from(disk, None)?;
    Ok(gpt.partitions()
        .iter()
        .enumerate()
        .map(|(i, part)| format!("GPT Partition {}: {:?}", i, part))
        .collect())
}

pub fn parse_mbr(disk: &mut std::fs::File) -> Result<Vec<String>, Box<dyn std::error::Error>> {
    let mbr = MBR::read_from(disk, 512)?;
    Ok(mbr.iter()
        .map(|(i, part)| format!("MBR Partition {}: {:?}", i, part))
        .collect())
}
