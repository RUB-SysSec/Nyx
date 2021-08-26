use core::ffi::c_void;
use nix::sys::mman::*;
use std::fs::File;
use std::os::unix::io::IntoRawFd;
use std::fmt;
//use std::sync::atomic::compiler_fence;
//use std::sync::atomic::Ordering;

use crate::nyx::mem_barrier::mem_barrier;

const AUX_BUFFER_SIZE: usize = 4096;

const AUX_MAGIC: u64 = 0x54502d554d4551_u64;
const QEMU_PT_VERSION: u16 = 1; /* let's start at 1 for the initial version using the aux buffer */

const HEADER_SIZE: usize = 128;
const CAP_SIZE: usize = 256;
const CONFIG_SIZE: usize = 512;
const STATE_SIZE: usize = 512;
//const MISC_SIZE: usize = 4096 - (HEADER_SIZE + CAP_SIZE + CONFIG_SIZE + STATE_SIZE);

const HEADER_OFFSET: usize = 0;
const CAP_OFFSET: usize = HEADER_OFFSET + HEADER_SIZE;
const CONFIG_OFFSET: usize = CAP_OFFSET + CAP_SIZE;
const STATE_OFFSET: usize = CONFIG_OFFSET + CONFIG_SIZE;
const MISC_OFFSET: usize = STATE_OFFSET + STATE_SIZE;
const MISC_SIZE: usize = AUX_BUFFER_SIZE - MISC_OFFSET;

#[derive(Debug)]
pub struct AuxBuffer {
    pub header: &'static mut auxilary_buffer_header_s,
    pub cap: &'static mut auxilary_buffer_cap_s,
    pub config: &'static mut auxilary_buffer_config_s,
    pub result: &'static mut auxilary_buffer_result_s,
    pub misc: &'static mut auxilary_buffer_misc_s,
}

impl AuxBuffer {
    pub fn new(file: File) -> Self {
        let prot = ProtFlags::PROT_READ | ProtFlags::PROT_WRITE;
        let flags = MapFlags::MAP_SHARED;
        unsafe {
            let ptr = mmap(0 as *mut c_void, 0x1000, prot, flags, file.into_raw_fd(), 0).unwrap();
            let header = (ptr.add(HEADER_OFFSET) as *mut auxilary_buffer_header_s)
                .as_mut()
                .unwrap();
            let cap = (ptr.add(CAP_OFFSET) as *mut auxilary_buffer_cap_s)
                .as_mut()
                .unwrap();
            let config = (ptr.add(CONFIG_OFFSET) as *mut auxilary_buffer_config_s)
                .as_mut()
                .unwrap();
            let result = (ptr.add(STATE_OFFSET) as *mut auxilary_buffer_result_s)
                .as_mut()
                .unwrap();
            let misc = (ptr.add(MISC_OFFSET) as *mut auxilary_buffer_misc_s)
                .as_mut()
                .unwrap();
            return Self {
                header,
                cap,
                config,
                result,
                misc,
            };
        }
    }

    pub fn validate_header(&self) {
        mem_barrier();
        let mgc = self.header.magic;
        assert_eq!(mgc, AUX_MAGIC);
        let version = self.header.version;
        assert_eq!(version, QEMU_PT_VERSION);
        let hash = self.header.hash;
        assert_eq!(hash, 61);
    }
}
#[derive(Debug, Copy, Clone)]
#[repr(C, packed(1))]
pub struct auxilary_buffer_header_s {
    pub magic: u64, /* 0x54502d554d4551 */
    pub version: u16,
    pub hash: u16,
}
#[derive(Debug, Copy, Clone)]
#[repr(C, packed(1))]
pub struct auxilary_buffer_cap_s {
    pub redqueen: u8,
    pub agent_timeout_detection: u8,
}
#[derive(Debug, Copy, Clone)]
#[repr(C, packed(1))]
pub struct auxilary_buffer_config_s {
    pub changed: u8, /* set this byte to kick in a rescan of this buffer */

    pub timeout_sec: u8,
    pub timeout_usec: u32,

    /* trigger to enable / disable different QEMU-PT modes */
    pub redqueen_mode: u8,
    pub trace_mode: u8,
    pub reload_mode: u8,
    pub verbose_level: u8,
    pub page_dump_mode: u8,
    pub page_addr: u64,
    /*  0 -> disabled
        1 -> decoding
        2 -> decoding + full disassembling
    */
    //uint8_t pt_processing_mode;
    pub protect_payload_buffer: u8,
}
#[derive(Debug, Copy, Clone)]
#[repr(C, packed(1))]
pub struct auxilary_buffer_result_s {
    /*  0 -> booting,
        1 -> loader level 1,
        2 -> loader level 2,
        3 -> ready to fuzz
    */
    pub state: u8,
    pub hprintf: u8,
    pub exec_done: u8,
    pub crash_found: u8,
    pub asan_found: u8,
    pub timeout_found: u8,
    pub reloaded: u8,
    pub pt_overflow: u8,

    pub runtime_usec: u32,
    pub runtime_sec: u8,

    pub page_not_found: u8,
    pub page_not_found_addr: u64,
    pub success: u8,
    pub payload_write_attempt_found: u8,
}

#[repr(C, packed(1))]
pub struct auxilary_buffer_misc_s {
    pub len: u16,
    pub data: [u8;MISC_SIZE-2],
}

fn inspect_bytes(bs: &[u8]) -> String {
    use std::ascii::escape_default;
    use std::str;

    let mut visible = String::new();
    for &b in bs {
        let part: Vec<u8> = escape_default(b).collect();
        visible.push_str(str::from_utf8(&part).unwrap());
    }
    visible
}
impl auxilary_buffer_misc_s{
    pub fn as_slice(&self) -> &[u8]{
        assert!(self.len as usize <= self.data.len());
        return &self.data[0..self.len as usize];
    }
    pub fn as_string(&self) -> String{
        inspect_bytes(self.as_slice())
    }
}

impl fmt::Debug for auxilary_buffer_misc_s {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("auxilary_buffer_misc_s")
         .field("data", &inspect_bytes(self.as_slice()))
         .finish()
    }
}