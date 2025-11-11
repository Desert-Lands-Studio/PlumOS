pub const PLAM_MAGIC: u32 = 0x504C414D;
pub const PLAM_CPU_X86_64: u16 = 0x8664;
pub const PLAM_CPU_ARM64: u16 = 0xAA64;
pub const PLAM_CPU_RISCV64: u16 = 0x00F3;
pub const PLAM_CPU_PRUM64: u16 = 0x7072;

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamHeader {
    pub magic: u32,
    pub version: u16,
    pub flags: u64,
    pub file_size: u64,
    pub hdr_crc32: u32,
    pub file_crc32: u32,
    pub format_version: u32,
    pub content_version: u32,
    pub image_base: u64,
    pub entry_offset: u64,
    pub stack_reserve: u64,
    pub stack_commit: u64,
    pub heap_reserve: u64,
    pub heap_commit: u64,
    pub cpu_id: u16,
    pub cpu_sub: u16,
    pub abi_version: u32,
    pub cpu_features: u64,
    pub os_abi: u32,
    pub os_version_min: u32,
    pub os_version_sdk: u32,
    pub str_table: PlamRva,
    pub sym_table: PlamRva,
    pub section_table_off: u64,
    pub section_count: u32,
    pub reloc_table_off: u64,
    pub reloc_count: u32,
    pub ph_off: u64,
    pub ph_count: u16,
    pub ph_entry_size: u16,
    pub resources: PlamRva,
    pub debug: PlamDebugInfo,
    pub uuid: [u8; 16],
    pub build_hash: [u8; 48],
    pub timestamp: u64,
    pub crypto_mode: u16,
    pub hash_type: u16,
    pub sig_scheme: u16,
    pub relro_level: u8,
    pub file_compression: u8,
    pub manifest: PlamRva,
    pub deps_count: u32,
    pub res_count: u32,
    pub lang_mask: u32,
    pub tool_major: u16,
    pub tool_minor: u16,
    pub tool_patch: u16,
    pub stdlib_ver: u16,
    pub comp_model: u8,
    pub lto_pgo_flags: u8,
    pub opt_level: u8,
    pub debug_level: u8,
    pub dirs: PlamDirectories,
    pub control_flow_start: u64,
    pub control_flow_size: u64,
    pub hotpatch_offset: u32,
    pub hotpatch_count: u32,
    pub subsystem_type: u16,
    pub subsystem_version: u16,
    pub subsystem_params: PlamSubsystemParams,
    pub subsystem_flags: u32,
    pub sec_flags: u16,
    pub sanitizer_level: u8,
    pub crypt_alg: u8,
    pub branch_prot: u8,
    pub prefetch_hint: u8,
    pub cache_align: u16,
    pub isolation_level: u8,
    pub namespace_flags: u8,
    pub container_features: u16,
    pub metadata_size: u64,
    pub build_info: PlamRva,
    pub api_constraints: PlamRva,
    pub code_size: u64,
    pub init_data_size: u64,
    pub uninit_data_size: u64,
    pub ext_hdr_off: u64,
    pub mem_regions: PlamRva,
    pub endian: u8,
    pub reserved: [u8; 63],
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamRva {
    pub off: u64,
    pub sz: u64,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamDebugInfo {
    pub debug: PlamRva,
    pub debug_type: u16,
    pub debug_version: u16,
    pub debug_size: u32,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamDirectories {
    pub security: PlamRva,
    pub loadcfg: PlamRva,
    pub tls: PlamRva,
    pub cfg: PlamRva,
    pub exceptions: PlamRva,
    pub basereloc: PlamRva,
    pub import_table: PlamRva,
    pub export_table: PlamRva,
    pub got: PlamRva,
    pub plt: PlamRva,
    pub subsystem_validator: PlamRva,
    pub fat_off: u64,
    pub fat_cnt: u32,
    pub fat_flags: u32,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamSubsystemParams {
    pub raw: [u8; 24],
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamSection {
    pub name_off: u64,
    pub type_: u32,
    pub flags: u32,
    pub addr: u64,
    pub offset: u64,
    pub size: u64,
    pub entsize: u64,
    pub align: u64,
    pub section_prot: u32,
    pub comp_alg: u32,
    pub comp_level: u32,
    pub entropy: u32,
    pub hash_offset: u64,
}

#[repr(C, packed)]
#[derive(Debug, Clone, Copy)]
pub struct PlamReloc {
    pub offset: u64,
    pub type_: u32,
    pub sym_idx: u32,
    pub addend: i64,
    pub accelerator: u32,
    pub reserved: u32,
}

pub fn load_plam(buffer: &[u8], base: usize) -> Result<usize, &'static str> {
    if buffer.len() < core::mem::size_of::<PlamHeader>() {
        return Err("PLAM: buffer too small for header");
    }

    if base % 4096 != 0 {
        return Err("PLAM: base address must be page-aligned");
    }

    let header = unsafe { &*(buffer.as_ptr() as *const PlamHeader) };

    if header.magic != PLAM_MAGIC {
        return Err("PLAM: invalid magic");
    }

    if header.version >> 8 != 3 {
        return Err("PLAM: unsupported version");
    }

    // --- Исправление: Проверка перекрытия без Vec ---
    if header.section_count > 0 {
        let sections = unsafe {
            core::slice::from_raw_parts(
                buffer.as_ptr().add(header.section_table_off as usize) as *const PlamSection,
                header.section_count as usize,
            )
        };

        // Проверяем каждый сегмент со всеми последующими
        for i in 0..sections.len() {
            let sec_a = &sections[i];
            let start_a = base + sec_a.addr as usize;
            let end_a = start_a + sec_a.size as usize;

            for j in (i + 1)..sections.len() {
                let sec_b = &sections[j];
                let start_b = base + sec_b.addr as usize;
                let end_b = start_b + sec_b.size as usize;

                if start_a < end_b && start_b < end_a {
                    return Err("PLAM: overlapping segments detected");
                }
            }
        }
    }

     if header.reloc_count > 0 {
        let reloc_off = header.reloc_table_off as usize;
        let reloc_size = core::mem::size_of::<PlamReloc>();
        let total_reloc_bytes = header.reloc_count as usize * reloc_size;

        if reloc_off + total_reloc_bytes > buffer.len() {
            return Err("PLAM: reloc table out of bounds"); // Лучше вернуть Result, а не паниковать
        }

        let relocs = unsafe {
            core::slice::from_raw_parts(
                buffer.as_ptr().add(reloc_off) as *const PlamReloc,
                header.reloc_count as usize,
            )
        };

        for rel in relocs {
            let place = base + rel.offset as usize;
            let value = base as u64 + rel.addend as u64;

            match rel.type_ {
                1 | 2 | 3 => {
                    unsafe {
                        let ptr = place as *mut u64;
                        *ptr += value;
                    }
                }
                _ => {}
            }
        }
    }

    Ok(base + header.entry_offset as usize)
}

pub struct PlamParser {
    base: *const u8,
}

impl PlamParser {
    pub fn new(base: *const u8) -> Self {
        Self { base }
    }

    pub fn validate(&self) -> bool {
        unsafe {
            let header = &*(self.base as *const PlamHeader);
            header.magic == 0x504C414D
        }
    }

    pub fn entry_point(&self) -> Result<usize, &'static str> {
        unsafe {
            let header = &*(self.base as *const PlamHeader);
            if !self.validate() {
                return Err("Invalid PLAM magic");
            }
            let entry = header.image_base as usize + header.entry_offset as usize;
            Ok(entry)
        }
    }

    pub fn load_segments(&self) {
        // Ключевой: Загрузка сегментов (loadable PT_LOAD). Stub: Предполагаем код после 4096.
    }
}