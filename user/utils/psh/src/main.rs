mod shell;
mod builtins;
mod terminal;

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    let mut shell = shell::Shell::new();
    shell.run().await
}