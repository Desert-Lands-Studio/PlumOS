#![no_std]
#![allow(non_camel_case_types, non_upper_case_globals)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
use core::mem::size_of;

pub const HEADER_MAX: usize = 4096;

/// Проверяем магию и архитектуру
pub unsafe fn header(blob: &[u8]) -> Option<&plam_header_t> {
    if blob.len() < size_of::<plam_header_t>() { return None; }
    let hdr = &*(blob.as_ptr() as *const plam_header_t);

    #[cfg(target_arch = "x86_64")]
    if hdr.magic != PLAM_MAGIC || hdr.cpu_id != plam_cpu_t::PLAM_CPU_X86_64 as u16 {
        return None;
    }
    Some(hdr)
}

/// Итератор по таблице секций (без Box, без alloc)
pub unsafe fn sections<'a>(
    hdr: &plam_header_t,
    blob: &'a [u8],
) -> impl Iterator<Item = &'a plam_section_t> {
    // Проверяем, что таблица помещается в файле
    let total = hdr.section_table_off
        .checked_add((hdr.section_count as u64) * size_of::<plam_section_t>() as u64)
        .and_then(|e| if e <= blob.len() as u64 { Some(e) } else { None })
        .unwrap_or(0);

    let effective_section_count = if total == 0 {
        0
    } else {
        hdr.section_count as usize
    };

    let base = blob.as_ptr().add(if total == 0 {
        0 // Не используется, так как количество секций 0
    } else {
        hdr.section_table_off as usize
    });

    (0..effective_section_count).map(move |i| {
        &*(base.add(i * size_of::<plam_section_t>()) as *const plam_section_t)
    })
}
