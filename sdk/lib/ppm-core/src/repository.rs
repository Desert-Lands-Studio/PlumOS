use crate::{Package, Channel, Architecture, Result};
use crate::package::PackageIndex;

#[derive(Debug, Clone)]
pub struct Repository {
    pub url: String,
    pub name: String,
    pub channel: Channel,
    pub architectures: Vec<Architecture>,
}

impl Repository {
    pub fn new(url: String, name: String, channel: Channel, architectures: Vec<Architecture>) -> Self {
        Self {
            url,
            name,
            channel,
            architectures,
        }
    }

    pub async fn fetch_index(&self) -> Result<PackageIndex> {
        Ok(PackageIndex {
            packages: Vec::new(),
            generated: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_secs()
                .to_string(),
            channel: self.channel,
        })
    }

    pub fn supported_architectures(&self) -> &[Architecture] {
        &self.architectures
    }

    pub fn find_package(&self, _name: &str, _version: Option<&str>) -> Result<Option<Package>> {
        Ok(None)
    }
}

pub struct RepositoryManager {
    repositories: Vec<Repository>,
}

impl RepositoryManager {
    pub fn new() -> Self {
        Self {
            repositories: Vec::new(),
        }
    }

    pub fn add_repository(&mut self, repo: Repository) {
        self.repositories.push(repo);
    }

    pub fn get_repositories(&self) -> &[Repository] {
        &self.repositories
    }

    pub fn find_package_across_repos(&self, name: &str, version: Option<&str>) -> Result<Option<Package>> {
        for repo in &self.repositories {
            if let Some(pkg) = repo.find_package(name, version)? {
                return Ok(Some(pkg));
            }
        }
        Ok(None)
    }
}