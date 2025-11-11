use crate::block::{BlockDevice, BlockError};
use core::ptr;
use core::sync::atomic::{fence, Ordering};
use log::{info, warn};

const SDHCI_DMA_ADDRESS: usize = 0x00;
const SDHCI_BLOCK_SIZE: usize = 0x04;
const SDHCI_BLOCK_COUNT: usize = 0x06;
const SDHCI_ARGUMENT: usize = 0x08;
const SDHCI_TRANSFER_MODE: usize = 0x0C;
const SDHCI_COMMAND: usize = 0x0E;
const SDHCI_RESPONSE_0: usize = 0x10;
const SDHCI_RESPONSE_1: usize = 0x14;
const SDHCI_RESPONSE_2: usize = 0x18;
const SDHCI_RESPONSE_3: usize = 0x1C;
const SDHCI_PRESENT_STATE: usize = 0x24;
const SDHCI_HOST_CONTROL_1: usize = 0x28;
const SDHCI_CLOCK_CONTROL: usize = 0x2C;
const SDHCI_SOFTWARE_RESET: usize = 0x2F;
const SDHCI_INT_STATUS: usize = 0x30;
const SDHCI_INT_ENABLE: usize = 0x34;

const CMD_GO_IDLE_STATE: u16 = 0;
const CMD_SEND_IF_COND: u16 = 8;
const CMD_APP_CMD: u16 = 55;
const ACMD_SD_SEND_OP_COND: u16 = 41;
const CMD_ALL_SEND_CID: u16 = 2;
const CMD_SEND_RELATIVE_ADDR: u16 = 3;
const CMD_SELECT_CARD: u16 = 7;
const CMD_SEND_CSD: u16 = 9;
const CMD_SET_BLOCKLEN: u16 = 16;
const CMD_READ_SINGLE_BLOCK: u16 = 17;
const CMD_READ_MULTIPLE_BLOCK: u16 = 18;
const CMD_WRITE_SINGLE_BLOCK: u16 = 24;
const CMD_WRITE_MULTIPLE_BLOCK: u16 = 25;
const CMD_STOP_TRANSMISSION: u16 = 12;

const SDHCI_CMD_RESP_NONE: u16 = 0x0000;
const SDHCI_CMD_RESP_R1: u16 = 0x0010;
const SDHCI_CMD_RESP_R2: u16 = 0x0018;
const SDHCI_CMD_RESP_R3: u16 = 0x0010;
const SDHCI_CMD_RESP_R6: u16 = 0x0010;
const SDHCI_CMD_CRC_EN: u16 = 0x0008;
const SDHCI_CMD_INDEX_EN: u16 = 0x0004;
const SDHCI_CMD_DATA: u16 = 0x0020;

const SDHCI_CMD_INHIBIT: u32 = 1 << 0;
const SDHCI_DATA_INHIBIT: u32 = 1 << 1;
const SDHCI_CARD_PRESENT: u32 = 1 << 16;

const SDHCI_INT_RESPONSE: u32 = 1 << 0;
const SDHCI_INT_DATA_END: u32 = 1 << 1;
const SDHCI_INT_DATA_CRC_ERR: u32 = 1 << 8;
const SDHCI_INT_DATA_TIMEOUT: u32 = 1 << 9;
const SDHCI_INT_ERROR: u32 = 1 << 15;

const SDHCI_CTRL_4BITBUS: u8 = 0x02;
const SDHCI_CLOCK_INT_EN: u16 = 1 << 0;
const SDHCI_CLOCK_INT_STABLE: u16 = 1 << 1;
const SDHCI_CLOCK_CARD_EN: u16 = 1 << 2;
const SDHCI_RESET_ALL: u8 = 0x01;

const BLOCK_SIZE: u16 = 512;
const MAX_RETRIES: usize = 1_000_000;

#[repr(align(128))]
struct AlignedBuffer([u8; 512]);

impl AlignedBuffer {
    const fn new() -> Self {
        Self([0; 512])
    }
}

#[derive(Debug, Clone, Copy)]
enum CardType {
    None,
    SDSC,
    SDHC,
    SDXC,
}

pub struct SdhciController {
    base: usize,
    rca: u32,
    card_type: CardType,
    block_count: u64,
}

impl SdhciController {
    pub const fn new(base: usize) -> Self {
        Self {
            base,
            rca: 0,
            card_type: CardType::None,
            block_count: 0,
        }
    }

    #[inline]
    fn read32(&self, reg: usize) -> u32 {
        let val = unsafe { ptr::read_volatile((self.base + reg) as *const u32) };
        fence(Ordering::Acquire);
        val
    }

    #[inline]
    fn write32(&self, reg: usize, val: u32) {
        fence(Ordering::Release);
        unsafe { ptr::write_volatile((self.base + reg) as *mut u32, val) };
    }

    #[inline]
    fn read16(&self, reg: usize) -> u16 {
        let val = unsafe { ptr::read_volatile((self.base + reg) as *const u16) };
        fence(Ordering::Acquire);
        val
    }

    #[inline]
    fn write16(&self, reg: usize, val: u16) {
        fence(Ordering::Release);
        unsafe { ptr::write_volatile((self.base + reg) as *mut u16, val) };
    }

    #[inline]
    fn read8(&self, reg: usize) -> u8 {
        let val = unsafe { ptr::read_volatile((self.base + reg) as *const u8) };
        fence(Ordering::Acquire);
        val
    }

    #[inline]
    fn write8(&self, reg: usize, val: u8) {
        fence(Ordering::Release);
        unsafe { ptr::write_volatile((self.base + reg) as *mut u8, val) };
    }

    fn wait_for_cmd(&self) -> Result<(), &'static str> {
        for _ in 0..MAX_RETRIES {
            if self.read32(SDHCI_PRESENT_STATE) & SDHCI_CMD_INHIBIT == 0 {
                return Ok(());
            }
        }
        Err("Timeout waiting for CMD inhibit")
    }

    fn wait_for_data(&self) -> Result<(), &'static str> {
        for _ in 0..MAX_RETRIES {
            if self.read32(SDHCI_PRESENT_STATE) & SDHCI_DATA_INHIBIT == 0 {
                return Ok(());
            }
        }
        Err("Timeout waiting for DATA inhibit")
    }

    fn wait_for_interrupt(&self, mask: u32) -> Result<u32, &'static str> {
        for _ in 0..MAX_RETRIES {
            let int_status = self.read32(SDHCI_INT_STATUS);
            
            if int_status & SDHCI_INT_DATA_CRC_ERR != 0 {
                return Err("SDHCI data CRC error");
            }
            if int_status & SDHCI_INT_DATA_TIMEOUT != 0 {
                return Err("SDHCI data timeout");
            }
            if int_status & (SDHCI_INT_ERROR & !(SDHCI_INT_DATA_CRC_ERR | SDHCI_INT_DATA_TIMEOUT)) != 0 {
                return Err("SDHCI hardware error");
            }
            
            if int_status & mask != 0 {
                self.write32(SDHCI_INT_STATUS, int_status);
                return Ok(int_status);
            }
        }
        Err("SDHCI operation timeout")
    }

    fn reset(&self) {
        self.write8(SDHCI_SOFTWARE_RESET, SDHCI_RESET_ALL);
        while self.read8(SDHCI_SOFTWARE_RESET) & SDHCI_RESET_ALL != 0 {}
    }

    fn init_clock(&self) -> Result<(), &'static str> {
        self.write16(SDHCI_CLOCK_CONTROL, SDHCI_CLOCK_INT_EN);
        
        for _ in 0..MAX_RETRIES {
            if self.read16(SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_INT_STABLE != 0 {
                break;
            }
        }
        
        let clk = self.read16(SDHCI_CLOCK_CONTROL);
        self.write16(SDHCI_CLOCK_CONTROL, clk | SDHCI_CLOCK_CARD_EN);
        
        Ok(())
    }

    fn set_high_speed(&self) {
        let clk = self.read16(SDHCI_CLOCK_CONTROL);
        self.write16(SDHCI_CLOCK_CONTROL, clk & !(0xFF << 8));
    }

    fn send_command(&self, cmd: u16, arg: u32, flags: u16) -> Result<u32, &'static str> {
        self.wait_for_cmd()?;
        self.write32(SDHCI_ARGUMENT, arg);
        self.write16(SDHCI_COMMAND, (cmd << 8) | flags);
        self.wait_for_interrupt(SDHCI_INT_RESPONSE)?;
        Ok(self.read32(SDHCI_RESPONSE_0))
    }

    fn is_card_present(&self) -> bool {
        self.read32(SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT != 0
    }

    fn parse_csd(&mut self, csd: [u32; 4]) -> Result<(), &'static str> {
        let csd_structure = (csd[0] >> 30) & 0x3;
        
        match csd_structure {
            0 => {
                let c_size = ((csd[1] & 0x3FF) << 2) | ((csd[2] >> 30) & 0x3);
                let c_size_mult = ((csd[2] >> 15) & 0x7) as u64;
                let read_bl_len = ((csd[1] >> 16) & 0xF) as u64;
                
                let block_len = 1u64 << read_bl_len;
                let mult = 1u64 << (c_size_mult + 2);
                let capacity = (c_size as u64 + 1) * mult * block_len;
                
                self.block_count = capacity / 512;
                self.card_type = CardType::SDSC;
            }
            1 => {
                let c_size = (((csd[1] & 0x3F) as u64) << 16) | ((csd[2] >> 16) as u64);
                self.block_count = (c_size + 1) * 1024;
                self.card_type = if self.block_count > 0x80000000 {
                    CardType::SDXC
                } else {
                    CardType::SDHC
                };
            }
            _ => return Err("Unknown CSD version"),
        }
        
        Ok(())
    }

    pub fn init(&mut self) -> Result<(), &'static str> {
        if !self.is_card_present() {
            return Err("No SD card present");
        }

        info!("SDHCI: Initializing controller at {:#x}", self.base);
        
        self.reset();
        self.init_clock()?;
        self.write32(SDHCI_INT_ENABLE, SDHCI_INT_RESPONSE | SDHCI_INT_DATA_END | SDHCI_INT_ERROR);

        self.send_command(CMD_GO_IDLE_STATE, 0, SDHCI_CMD_RESP_NONE)?;

        let resp = self.send_command(
            CMD_SEND_IF_COND, 
            0x1AA, 
            SDHCI_CMD_RESP_R1 | SDHCI_CMD_CRC_EN | SDHCI_CMD_INDEX_EN
        )?;
        
        if resp & 0xFFF != 0x1AA {
            warn!("SDHCI: Card doesn't support required voltage");
        }

        let mut ocr = 0x40300000;
        for _ in 0..1000 {
            self.send_command(CMD_APP_CMD, 0, SDHCI_CMD_RESP_R1)?;
            let resp = self.send_command(ACMD_SD_SEND_OP_COND, ocr, SDHCI_CMD_RESP_R3)?;
            if resp & (1 << 31) != 0 {
                ocr = resp;
                break;
            }
        }

        if ocr & (1 << 31) == 0 {
            return Err("Card initialization timeout");
        }

        if ocr & (1 << 30) != 0 {
            self.card_type = CardType::SDHC;
        }

        self.send_command(CMD_ALL_SEND_CID, 0, SDHCI_CMD_RESP_R2)?;
        let rca_resp = self.send_command(CMD_SEND_RELATIVE_ADDR, 0, SDHCI_CMD_RESP_R6)?;
        self.rca = (rca_resp >> 16) & 0xFFFF;
        
        if self.rca == 0 {
            return Err("Invalid RCA received");
        }

        self.send_command(CMD_SELECT_CARD, self.rca << 16, SDHCI_CMD_RESP_R1)?;

        let csd_resp = [
            self.send_command(CMD_SEND_CSD, self.rca << 16, SDHCI_CMD_RESP_R2)?,
            self.read32(SDHCI_RESPONSE_1),
            self.read32(SDHCI_RESPONSE_2),
            self.read32(SDHCI_RESPONSE_3),
        ];
        
        self.parse_csd(csd_resp)?;
        self.send_command(CMD_SET_BLOCKLEN, BLOCK_SIZE as u32, SDHCI_CMD_RESP_R1)?;
        self.set_high_speed();

        let mut host_ctrl = self.read8(SDHCI_HOST_CONTROL_1);
        host_ctrl |= SDHCI_CTRL_4BITBUS;
        self.write8(SDHCI_HOST_CONTROL_1, host_ctrl);

        info!("SDHCI: Card initialized (RCA: {:#x}, Type: {:?}, Blocks: {})",
            self.rca, self.card_type, self.block_count);

        Ok(())
    }

    fn transfer_blocks(&self, lba: u64, buffer: &mut [u8], write: bool) -> Result<(), &'static str> {
        let block_count = (buffer.len() / 512) as u16;
        if block_count == 0 || buffer.len() % 512 != 0 {
            return Err("Buffer size must be multiple of 512 bytes");
        }

        if buffer.as_ptr() as usize % 4 != 0 {
            return Err("Buffer must be 4-byte aligned for DMA");
        }

        let addr = match self.card_type {
            CardType::SDSC => lba * 512,
            _ => lba,
        };

        self.wait_for_data()?;
        self.write16(SDHCI_BLOCK_SIZE, BLOCK_SIZE);
        self.write16(SDHCI_BLOCK_COUNT, block_count);

        let transfer_mode = 0x0002 | if block_count > 1 { 0x0020 } else { 0x0000 };
        self.write16(SDHCI_TRANSFER_MODE, transfer_mode);

        let dma_addr = buffer.as_ptr() as u32;
        self.write32(SDHCI_DMA_ADDRESS, dma_addr);

        let cmd = match (write, block_count > 1) {
            (false, true) => CMD_READ_MULTIPLE_BLOCK,
            (false, false) => CMD_READ_SINGLE_BLOCK,
            (true, true) => CMD_WRITE_MULTIPLE_BLOCK,
            (true, false) => CMD_WRITE_SINGLE_BLOCK,
        };

        let cmd_flags = SDHCI_CMD_RESP_R1 | SDHCI_CMD_CRC_EN | SDHCI_CMD_INDEX_EN | SDHCI_CMD_DATA;
        self.send_command(cmd, addr as u32, cmd_flags)?;

        self.wait_for_interrupt(SDHCI_INT_DATA_END)?;

        if !write && block_count > 1 {
            self.send_command(CMD_STOP_TRANSMISSION, 0, SDHCI_CMD_RESP_R1)?;
        }

        Ok(())
    }
}

impl BlockDevice for SdhciController {
    fn read_blocks(&mut self, lba: u64, buffer: &mut [u8]) -> Result<(), BlockError> {
        self.transfer_blocks(lba, buffer, false).map_err(|_| BlockError::IoError)
    }
    fn write_blocks(&mut self, lba: u64, buffer: &[u8]) -> Result<(), BlockError> {
        if buffer.len() % 512 != 0 {
            return Err(BlockError::InvalidParameter);
        }
        let mut dma_buffer = AlignedBuffer::new();
        for (i, chunk) in buffer.chunks(512).enumerate() {
            dma_buffer.0[..chunk.len()].copy_from_slice(chunk);
            let block_lba = lba + i as u64;
            let dma_slice = unsafe {
                core::slice::from_raw_parts_mut(
                    &mut dma_buffer.0 as *mut [u8; 512] as *mut u8,
                    512,
                )
            };
            self.transfer_blocks(block_lba, dma_slice, true)
                .map_err(|_| BlockError::IoError)?;
        }
        Ok(())
    }
    fn block_size(&self) -> usize { BLOCK_SIZE as usize }
    fn block_count(&self) -> u64 { self.block_count }
}