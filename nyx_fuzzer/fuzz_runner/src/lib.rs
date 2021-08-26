extern crate byteorder;
extern crate glob;
extern crate nix;
extern crate serde_derive;
extern crate snafu;
extern crate tempfile;
extern crate timeout_readwrite;
extern crate config;
#[macro_use] extern crate lazy_static;
extern crate regex;
extern crate hex;


pub mod exitreason;
pub use exitreason::ExitReason;

pub mod forksrv;
pub use forksrv::ForkServer;

pub mod nyx;
pub use nyx::QemuProcess;

use std::error::Error;


#[derive(Debug,Clone,Eq,PartialEq,Hash)]
pub struct TestInfo { pub exitreason: ExitReason }

#[derive(Debug,Clone,Eq,PartialEq,Hash)]
pub enum RedqueenBPType{
    Str,
    Cmp,
    Sub,
}

impl RedqueenBPType{
    pub fn new(data:&str) -> Self {
        match data {
            "STR" => return Self::Str,
            "CMP" => return Self::Cmp,
            "SUB" => return Self::Sub,
            _ => panic!("unknown reqdueen bp type {}",data),
        }
    }
}

#[derive(Debug,Clone,Eq,PartialEq,Hash)]
pub struct RedqueenEvent{
    pub addr: u64,
    pub bp_type: RedqueenBPType,
    pub lhs: Vec<u8>,
    pub rhs: Vec<u8>,
    pub imm: bool,
}

impl RedqueenEvent{
    pub fn new(line: &str) -> Self{
        lazy_static! {
            static ref RE : regex::Regex = regex::Regex::new(r"([0-9a-fA-F]+)\s+(CMP|SUB|STR)\s+(\d+)\s+([0-9a-fA-F]+)-([0-9a-fA-F]+)(\sIMM)?").unwrap();
        }
        if let Some(mat) = RE.captures(line){
            let addr_s = mat.get(1).unwrap().as_str();
            let type_s = mat.get(2).unwrap().as_str();
            let bits_s =mat.get(3);
            let lhs = mat.get(4).unwrap().as_str();
            let rhs = mat.get(5).unwrap().as_str();
            let imm = mat.get(6).map(|x| true).unwrap_or(false);
            return Self{addr: u64::from_str_radix(addr_s, 16).unwrap(), bp_type: RedqueenBPType::new(type_s), lhs: hex::decode(lhs).unwrap(), rhs: hex::decode(rhs).unwrap(), imm};
        }
        panic!("couldn't parse redqueen line {}",line); 
    }
}

#[derive(Debug,Clone,Eq,PartialEq,Hash)]
pub struct RedqueenInfo {pub bps: Vec<RedqueenEvent>}

pub struct CFGInfo {}

pub trait FuzzRunner {
    fn run_test(&mut self) -> Result<TestInfo, Box<dyn Error>>;
    fn run_redqueen(&mut self) -> Result<RedqueenInfo, Box<dyn Error>>;
    fn run_cfg(&mut self) -> Result<CFGInfo, Box<dyn Error>>;

    fn shutdown(self) -> Result<(), Box<dyn Error>>;

    fn input_buffer(&mut self) -> &mut [u8];
    fn bitmap_buffer(&self) -> &[u8];
    fn set_input_size(&mut self, size: usize);

    fn parse_redqueen_data(&self, data: &str) -> RedqueenInfo{
        let bps = data.lines().map(|line| RedqueenEvent::new(line)).collect::<Vec<_>>();
        return RedqueenInfo{bps}
    }
    fn parse_redqueen_file(&self, path: &str) -> RedqueenInfo{
        self.parse_redqueen_data(&std::fs::read_to_string(path).unwrap())
    }
}

impl FuzzRunner for ForkServer {
    fn run_test(&mut self) -> Result<TestInfo, Box<dyn Error>> {
        self.run().unwrap();

        return Ok(TestInfo {exitreason: ExitReason::FuzzerError}); //TODO fix this!
    }

    fn run_redqueen(&mut self) -> Result<RedqueenInfo, Box<dyn Error>> {
        unreachable!();
        //return Ok(parse_redqueen_file());
    }

    fn run_cfg(&mut self) -> Result<CFGInfo, Box<dyn Error>> {
        return Ok(CFGInfo {});
    }

    fn shutdown(self) -> Result<(), Box<dyn Error>> {
        return Ok(());
    }
    fn input_buffer(&mut self) -> &mut [u8] {
        self.get_input_mut()
    }
    fn bitmap_buffer(&self) -> &[u8] {
        self.get_bitmap()
    }
    fn set_input_size(&mut self, size: usize) {
        ForkServer::set_input_size(self, size)
    }
}

impl FuzzRunner for QemuProcess {
    fn run_test(&mut self) -> Result<TestInfo, Box<dyn Error>> {
        self.send_payload();
        if self.aux.result.crash_found != 0 {
            return Ok(TestInfo {exitreason: ExitReason::Crash(self.aux.misc.as_slice().to_vec())});
        }
        if self.aux.result.payload_write_attempt_found != 0{
            return Ok(TestInfo {exitreason: ExitReason::InvalidWriteToPayload(self.aux.misc.as_slice().to_vec())});
        }
        if self.aux.result.timeout_found != 0 {
            return Ok(TestInfo {exitreason: ExitReason::Timeout});
        }
        if self.aux.result.asan_found != 0 {
            return Ok(TestInfo {exitreason: ExitReason::Asan});
        }
        if self.aux.result.success != 0{
            return Ok(TestInfo {exitreason: ExitReason::Normal(0)});
        }
        println!("unknown exeuction result!!");
        return Ok(TestInfo {exitreason: ExitReason::FuzzerError});
    }

    fn run_redqueen(&mut self) -> Result<RedqueenInfo, Box<dyn Error>> {
        self.aux.config.changed = 1;
        self.aux.config.redqueen_mode=1;
        self.send_payload();
        self.aux.config.changed = 1;
        self.aux.config.redqueen_mode=0;
        let rq_file = format!("{}/redqueen_workdir_{}/redqueen_results.txt",self.params.workdir,self.params.qemu_id);
        return Ok(self.parse_redqueen_file(&rq_file));
    }

    fn run_cfg(&mut self) -> Result<CFGInfo, Box<dyn Error>> {
        //println!("TRACE!!!!");
        self.aux.config.trace_mode=1;
        self.aux.config.changed = 1;
        self.send_payload();
        self.aux.config.changed = 1;
        self.aux.config.trace_mode=0;
        return Ok(CFGInfo {});
    }

    fn shutdown(self) -> Result<(), Box<dyn Error>> {
        return Ok(());
    }
    fn input_buffer(&mut self) -> &mut [u8] {
        &mut self.payload[..]
    }
    fn bitmap_buffer(&self) -> &[u8] {
        self.bitmap
    }
    fn set_input_size(&mut self, _size: usize) {
        //self.payload[4..].copy_from_slice(&(size as u32).to_le_bytes());
    }
}
