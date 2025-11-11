#[cfg(target_arch = "aarch64")]
pub mod pl011;
#[cfg(target_arch = "x86_64")]
pub mod ns16550;
#[cfg(target_arch = "riscv64")]
pub mod uart16550;

#[cfg(target_arch = "aarch64")]
pub use pl011::Pl011Uart as DefaultUart;
#[cfg(target_arch = "x86_64")]
pub use ns16550::Ns16550Uart as DefaultUart;
#[cfg(target_arch = "riscv64")]
pub use uart16550::Uart16550 as DefaultUart;

use crate::Uart;
use spin::Mutex;
use crate::HalResult;

#[cfg(target_arch = "x86_64")]
static DEFAULT_UART: Mutex<Option<DefaultUart>> = Mutex::new(None);

#[cfg(target_arch = "x86_64")]
pub fn init_default_uart() -> HalResult<()> {
    let mut uart = unsafe { DefaultUart::new(0x3F8) };
    uart.init()?;
    *DEFAULT_UART.lock() = Some(uart);
    Ok(())
}

#[cfg(not(target_arch = "x86_64"))]
pub fn init_default_uart() -> HalResult<()> {
    Ok(())
}