pub mod config {
pub const ARCH: &str = "aarch64";
pub const PAGE_SIZE: usize = 4096;
pub const SMP_CORES: usize = 6;
pub const SECURE_BOOT: bool = true;
pub const DRIVER_USB: bool = true;
pub const DRIVER_WIFI: bool = true;
pub const DRIVER_GPU: bool = true;
pub const DRIVER_NVME: bool = false;
pub const ABI_POSIX: bool = true;
pub const ABI_WIN32: bool = true;
pub const ABI_DARWIN: bool = true;
pub const ABI_ANDROID: bool = true;
pub const BUILD_MODE: &str = "release";
pub const PLAM_SUBSYSTEM: &str = "native_kernel";
pub const PLAM_FLAGS: &[&str] = &[];
}
