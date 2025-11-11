use ppm_core::Package;

pub async fn analyze_package(package: &Package) -> Result<bool, Box<dyn std::error::Error>> {
    
    println!("🤖 AI analysis of package {}...", package.name);
    
    tokio::time::sleep(tokio::time::Duration::from_secs(2)).await;
    
    let suspicious_keywords = ["crack", "keygen", "hack", "exploit"];
    let name_lower = package.name.to_lowercase();
    
    for keyword in suspicious_keywords {
        if name_lower.contains(keyword) {
            return Ok(true);
        }
    }
    
    Ok(false)
}