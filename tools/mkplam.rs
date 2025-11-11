use std::env;
use std::fs::File;
use std::io::{Read, Write};
use std::process::exit;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 4 {
        print_usage();
        exit(1);
    }

    let mut input_path = None;
    let mut output_path = None;
    let mut cpu_id = None;

    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "--arch=aarch64" => cpu_id = Some(0xAA64u16),
            "--arch=x86_64" => cpu_id = Some(0x8664u16),
            "--arch=riscv64" => cpu_id = Some(0x00F3u16),
            "--arch=prum64" => cpu_id = Some(0x7072u16),
            _ if input_path.is_none() => input_path = Some(args[i].clone()),
            _ if output_path.is_none() => output_path = Some(args[i].clone()),
            _ => {
                eprintln!("❌ Unknown argument: {}", args[i]);
                print_usage();
                exit(1);
            }
        }
        i += 1;
    }

    let input_path = input_path.expect("Missing input file");
    let output_path = output_path.expect("Missing output file");
    let cpu_id = cpu_id.expect("Missing architecture");

    // Проверка существования входного файла
    if !std::path::Path::new(&input_path).exists() {
        eprintln!("❌ Input file not found: {}", input_path);
        exit(1);
    }

    let mut raw_data = Vec::new();
    File::open(&input_path)
        .and_then(|mut f| f.read_to_end(&mut raw_data))
        .unwrap_or_else(|e| {
            eprintln!("❌ Failed to read {}: {}", input_path, e);
            exit(1);
        });

    // Проверка размера файла
    if raw_data.len() > 1024 * 1024 * 1024 { // 1GB limit
        eprintln!("❌ Input file too large (max 1GB)");
        exit(1);
    }

    // Проверка выравнивания
    if raw_data.len() % 8 != 0 {
        println!("⚠️  Input file size not 8-byte aligned, adding padding");
        let padding = 8 - (raw_data.len() % 8);
        raw_data.extend(vec![0u8; padding]);
    }

    let code_size = raw_data.len() as u64;
    let (image_base, architecture_name) = match cpu_id {
        0xAA64 => (0x4008_0000, "AArch64"),   // AArch64 (QEMU virt)
        0x8664 => (0x100_000, "x86_64"),      // x86_64
        0x00F3 => (0x8000_0000, "RISC-V 64"), // RISC-V (стандартный base для QEMU virt)
        0x7072 => (0x8000_0000, "prum64"),    // prum64
        _ => {
            eprintln!("❌ Unknown CPU architecture: 0x{:04X}", cpu_id);
            exit(1);
        }
    };

    let entry_offset: u64 = 0;
    let mut header = Vec::with_capacity(4096);

    // Сигнатура PLAM
    header.extend_from_slice(b"PLAM");
    header.extend_from_slice(&((3 << 8) | 0u16).to_le_bytes()); // v3.0
    header.extend_from_slice(&[0u8; 6]); // reserved
    
    // Флаги
    let flags = 0u64; // Базовые флаги
    header.extend_from_slice(&flags.to_le_bytes());

    // File size
    let file_size = 4096 + code_size;
    header.extend_from_slice(&file_size.to_le_bytes());

    // Reserved
    header.extend_from_slice(&[0u8; 32]);

    // Image base
    header.extend_from_slice(&image_base.to_le_bytes());

    // Entry offset
    header.extend_from_slice(&entry_offset.to_le_bytes());

    // CPU ID (на позиции 0x18, как в plam_header_t)
    while header.len() < 0x18 {
        header.push(0);
    }
    header.extend_from_slice(&cpu_id.to_le_bytes());

    // Заполняем до 4096 байт нулями
    while header.len() < 4096 {
        header.push(0);
    }

    // Создаём PLAM файл
    let mut out_file = File::create(&output_path).unwrap_or_else(|e| {
        eprintln!("❌ Failed to create {}: {}", output_path, e);
        exit(1);
    });

    // Записываем заголовок и данные
    if let Err(e) = out_file.write_all(&header) {
        eprintln!("❌ Failed to write header: {}", e);
        exit(1);
    }
    
    if let Err(e) = out_file.write_all(&raw_data) {
        eprintln!("❌ Failed to write data: {}", e);
        exit(1);
    }

    // Вывод информации о созданном файле
    println!("✅ Created {} ({} bytes)", output_path, 4096 + raw_data.len());
    println!("   - Architecture: {}", architecture_name);
    println!("   - Image base: 0x{:x}", image_base);
    println!("   - Entry offset: 0x{:x}", entry_offset);
    println!("   - Code size: {} bytes", code_size);
    println!("   - Total size: {} bytes", file_size);
    
    // Проверка CRC32 (опционально)
    if let Ok(metadata) = std::fs::metadata(&output_path) {
        println!("   - File size on disk: {} bytes", metadata.len());
    }
}

fn print_usage() {
    eprintln!("Usage: mkplam <input.raw> <output.plam> --arch=aarch64|--arch=x86_64|--arch=riscv64|--arch=prum64");
    eprintln!();
    eprintln!("Examples:");
    eprintln!("  mkplam kernel.bin kernel.plam --arch=x86_64");
    eprintln!("  mkplam bootloader.bin bootloader.plam --arch=aarch64");
}