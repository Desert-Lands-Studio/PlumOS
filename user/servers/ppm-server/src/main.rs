use warp::Filter;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::Mutex;
use ppm_core::{Package, Channel, Architecture, PackageIndex};
use serde::{Deserialize, Serialize};
use std::fs;

mod antivirus;
mod ai_scanner;

type StagingMap = Arc<Mutex<HashMap<String, Package>>>;

#[derive(Serialize, Deserialize)]
struct ApproveRequest {
    package_key: String,
    target_channel: String,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let staging: StagingMap = Arc::new(Mutex::new(HashMap::new()));

    let staging_clone = staging.clone();
    let upload = warp::post()
        .and(warp::path("upload"))
        .and(warp::body::json())
        .and(with_staging(staging_clone))
        .and_then(upload_handler);

    let staging_clone = staging.clone();
    let list_staging = warp::get()
        .and(warp::path("staging"))
        .and(with_staging(staging_clone))
        .and_then(list_staging_handler);

    let staging_clone = staging.clone();
    let approve = warp::post()
        .and(warp::path("approve"))
        .and(warp::body::json::<ApproveRequest>())
        .and(with_staging(staging_clone))
        .and_then(approve_handler);

    let api = warp::path("api")
        .and(upload.or(list_staging).or(approve));

    let web = warp::path("admin")
        .and(warp::fs::dir("/srv/ppm/web/admin"));

    let routes = api.or(web).with(
        warp::cors()
            .allow_any_origin()
            .allow_methods(vec!["GET", "POST"])
            .allow_headers(vec!["content-type"]),
    );

    println!("🚀 PPM Admin running on http://localhost:8080/admin");
    warp::serve(routes).run(([0, 0, 0, 0], 8080)).await?;
    Ok(())
}

fn with_staging(
    staging: StagingMap,
) -> impl Filter<Extract = (StagingMap,), Error = std::convert::Infallible> + Clone {
    warp::any().map(move || staging.clone())
}

async fn upload_handler(
    mut package: Package,
    staging: StagingMap,
) -> Result<impl warp::Reply, warp::Rejection> {
    if package.architecture == Architecture::Prum64 {
        package.architecture = Architecture::current();
    }

    // 🔍 Антивирус
    if let Ok(true) = antivirus::scan_package(&package).await {
        return Ok(warp::reply::json(&serde_json::json!({
            "error": "rejected",
            "reason": "virus_detected"
        })));
    }

    // 🤖 ИИ-анализ
    if let Ok(true) = ai_scanner::analyze_package(&package).await {
        return Ok(warp::reply::json(&serde_json::json!({
            "error": "flagged",
            "reason": "suspicious_content"
        })));
    }

    let key = format!(
        "{}-{}-{}",
        package.name,
        package.version,
        package.architecture.as_str()
    );

    // 📁 Сохраняем JSON-метаданные в pending
    let meta_path = format!("/srv/ppm/staging/pending/{}.meta.json", key);
    let meta = serde_json::json!({
        "filename": format!("{}.plpm", key),
        "channel": "pending",
        "verified": false,
        "note": "awaiting human review"
    });
    fs::create_dir_all("/srv/ppm/staging/pending").ok();
    fs::write(&meta_path, serde_json::to_string_pretty(&meta).unwrap()).ok();

    // 📦 Сохраняем в памяти для API
    staging.lock().await.insert(key.clone(), package);

    Ok(warp::reply::json(&serde_json::json!({
        "status": "pending_review",
        "key": key
    })))
}

async fn list_staging_handler(
    staging: StagingMap,
) -> Result<impl warp::Reply, warp::Rejection> {
    let packages: Vec<serde_json::Value> = {
        let lock = staging.lock().await;
        lock.values()
            .map(|pkg| {
                serde_json::json!({
                    "key": format!(
                        "{}-{}-{}",
                        pkg.name,
                        pkg.version,
                        pkg.architecture.as_str()
                    ),
                    "name": pkg.name,
                    "version": pkg.version,
                    "channel": pkg.channel.name(),
                    "architecture": pkg.architecture.as_str(),
                    "author": pkg.author,
                    "description": pkg.description,
                    "size": pkg.size,
                })
            })
            .collect()
    };
    Ok(warp::reply::json(&packages))
}

async fn approve_handler(
    req: ApproveRequest,
    staging: StagingMap,
) -> Result<impl warp::Reply, warp::Rejection> {
    let target_channel = match req.target_channel.as_str() {
        "stable" => Channel::Stable,
        "testing" => Channel::Testing,
        "unstable" => Channel::Unstable,
        _ => {
            return Ok(warp::reply::json(&serde_json::json!({
                "error": "invalid channel"
            })))
        }
    };

    let mut lock = staging.lock().await;
    if let Some(pkg) = lock.remove(&req.package_key) {
        // 📂 Путь назначения
        let arch_dir = format!(
            "/srv/ppm/{}/bin/{}",
            target_channel.name(),
            pkg.architecture.as_str()
        );
        fs::create_dir_all(&arch_dir).ok();

        // ✍️ Обновляем index.json
        let index_path = format!("{}/index.json", arch_dir);
        let mut index: PackageIndex = if fs::metadata(&index_path).is_ok() {
            let data = fs::read_to_string(&index_path).unwrap_or_default();
            serde_json::from_str(&data)
                .unwrap_or_else(|_| PackageIndex {
                    packages: vec![],
                    generated: "".to_string(),
                    channel: Channel::Dev,
                })
        } else {
            PackageIndex {
                packages: vec![],
                generated: "".to_string(),
                channel: target_channel,
            }
        };

        if let Some(pos) = index.packages.iter().position(|p| p.name == pkg.name) {
            index.packages[pos] = pkg.clone();
        } else {
            index.packages.push(pkg.clone());
        }

        index.generated = chrono::Utc::now().to_rfc3339();
        index.channel = target_channel;
        fs::write(&index_path, serde_json::to_string_pretty(&index).unwrap()).ok();

        println!(
            "✅ Approved {} → {}/{}",
            req.package_key,
            target_channel.name(),
            pkg.architecture.as_str()
        );

        Ok(warp::reply::json(&serde_json::json!({
            "status": "approved",
            "channel": target_channel.name()
        })))
    } else {
        Ok(warp::reply::json(&serde_json::json!({
            "error": "package not found"
        })))
    }
}