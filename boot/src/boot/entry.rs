// src/boot/entry.rs
use core::arch::asm;

pub unsafe fn jump_to_entry(entry_point: u64) -> ! {
    asm!(
        "jmp {0}",
        in(reg) entry_point,
        options(noreturn)
    );
}
