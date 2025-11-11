use core::sync::atomic::{AtomicU32, Ordering};

static ENABLED_FEATURES: AtomicU32 = AtomicU32::new(0);

#[repr(u32)]
#[derive(Clone, Copy)]
pub enum Feature {
    Graphics = 1 << 0,
    Network = 1 << 1,
    SecureBoot = 1 << 2,
    FileSystems = 1 << 3,
    Multiboot = 1 << 4,
    UEFI = 1 << 5,
}

impl Feature {
    pub fn enable(self) {
        ENABLED_FEATURES.fetch_or(self as u32, Ordering::SeqCst);
    }

    pub fn disable(self) {
        ENABLED_FEATURES.fetch_and(!(self as u32), Ordering::SeqCst);
    }

    pub fn is_enabled(self) -> bool {
        ENABLED_FEATURES.load(Ordering::SeqCst) & (self as u32) != 0
    }
}

#[macro_export]
macro_rules! with_feature {
    ($feature:expr, $code:block) => {
        if $feature.is_enabled() {
            $code
        }
    };
}

pub fn init_features() {
    Feature::FileSystems.enable();
    Feature::Multiboot.enable();
}
