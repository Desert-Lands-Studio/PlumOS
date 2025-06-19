use std::{env, path::PathBuf};

fn main() {
    // Указываем, когда пересобирать
    println!("cargo:rerun-if-changed=../../include/plam_header.h");

    // Генерируем биндинги
    let bindings = bindgen::Builder::default()
        .header("../../include/plam_header.h")
        .use_core() // Для no_std
        .layout_tests(false) // Отключаем тесты
        .allowlist_type("plam_.*")
        .allowlist_var("PLAM_.*")
        .clang_arg("-Wno-pragma-pack")
        .clang_arg("-I/usr/include/x86_64-linux-gnu")
        // Подавляем предупреждения о стиле именования
        .rustified_enum("plam_cpu_t")
        .rustified_enum("plam_cpu_subtype_t")
        .rustified_enum("plam_file_type_t")
        .rustified_enum("plam_res_type_t")
        .generate()
        .expect("Unable to generate bindings");

    // Сохраняем в $OUT_DIR
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}