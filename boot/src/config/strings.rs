// src/config/strings.rs
static STRINGS_EN: Strings = Strings {
    choose_os: "Select OS to boot:",
    loading: "Loading...",
    error: "Error loading kernel",
    // ...
};
static STRINGS_RU: Strings = Strings {
    choose_os: "Выберите ОС для загрузки:",
    loading: "Загрузка...",
    error: "Ошибка загрузки ядра",
    // ...
};

struct Strings {
    choose_os: &'static str,
    loading: &'static str,
    error: &'static str,
    // ...
}

// Текущие выбранные строки (по умолчанию англ)
static mut CURR: &Strings = &STRINGS_EN;

pub fn select_locale(lang: &str) {
    unsafe {
        CURR = if lang.starts_with("ru") {
            &STRINGS_RU
        } else {
            &STRINGS_EN
        };
    }
}
pub fn s() -> &'static Strings {
    unsafe { CURR }
}
