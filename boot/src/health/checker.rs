// src/health/checker.rs
use uefi::table::boot::MemoryType;
use uefi::table::SystemTable;

pub fn check_system_health(st: &SystemTable<uefi::table::Boot>) {
    let memory_map = st.boot_services().memory_map().unwrap();
    let total_memory_bytes: u64 = memory_map
        .entries()
        .iter()
        .filter(|entry| entry.ty == MemoryType::CONVENTIONAL)
        .map(|entry| entry.page_count * 4096)
        .sum();

    println!("Available memory: {} bytes", total_memory_bytes);
}
