use core::ffi::c_void;
use nix::sys::mman::*;
use std::fs;
use std::fs::{File, OpenOptions};
use std::io::prelude::*;
use std::os::unix::fs::symlink;
use std::os::unix::io::IntoRawFd;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::process::Child;
use std::process::Command;
use std::{thread, time};


use crate::nyx::aux_buffer::AuxBuffer;
use crate::nyx::mem_barrier::mem_barrier;
use crate::nyx::params::QemuParams;

pub struct QemuProcess {
    pub process: Child,
    pub aux: AuxBuffer,
    pub ctrl: UnixStream,
    pub bitmap: &'static mut [u8],
    pub payload: &'static mut [u8],
    pub params: QemuParams,
}

fn execute_qemu(ctrl: &mut UnixStream) {
    ctrl.write(&[120_u8]).unwrap();
}

fn wait_qemu(ctrl: &mut UnixStream) {
    let mut buf = [0];
    ctrl.read(&mut buf).unwrap();
}

fn run_qemu(ctrl: &mut UnixStream) {
    execute_qemu(ctrl);
    wait_qemu(ctrl);
}

fn make_shared_data(file: File, size: usize) -> &'static mut [u8] {
    let prot = ProtFlags::PROT_READ | ProtFlags::PROT_WRITE;
    let flags = MapFlags::MAP_SHARED;
    unsafe {
        let ptr = mmap(0 as *mut c_void, size, prot, flags, file.into_raw_fd(), 0).unwrap();

        let data = std::slice::from_raw_parts_mut(ptr as *mut u8, size);
        return data;
    }
}

impl QemuProcess {
    pub fn new(params: QemuParams) -> QemuProcess {
        Self::prepare_redqueen_workdir(&params.workdir, params.qemu_id);
        println!("Spawingn qemu with:\n {}", params.cmd.join(" "));
        let child = Command::new(&params.cmd[0])
            .args(&params.cmd[1..])
            .spawn()
            .expect("failed to execute process");

        thread::sleep(time::Duration::from_secs(1));

        println!("CONNECT TO {}", params.control_filename);

        //control.settimeout(None) maybe needed?
        //control.setblocking(1)

        let mut control = loop {
            match UnixStream::connect(&params.control_filename) {
                Ok(stream) => break stream,
                _ => thread::sleep(time::Duration::from_millis(1)),
            }
        };

        let bitmap_shm_f = OpenOptions::new()
            .create(true)
            .read(true)
            .write(true)
            .open(&params.bitmap_filename)
            .expect("couldn't open bitmap file");
        let mut payload_shm_f = OpenOptions::new()
            .create(true)
            .read(true)
            .write(true)
            .open(&params.payload_filename)
            .expect("couldn't open payload file");

        symlink(
            &params.bitmap_filename,
            format!("{}/bitmap_{}", params.workdir, params.qemu_id),
        )
        .unwrap();
        symlink(
            &params.payload_filename,
            format!("{}/payload_{}", params.workdir, params.qemu_id),
        )
        .unwrap();
        println!("======================================SET NOT_INIT!!!!");
        payload_shm_f.write(b"not_init").unwrap();
        bitmap_shm_f.set_len(params.bitmap_size as u64).unwrap();
        payload_shm_f.set_len(params.payload_size as u64).unwrap();

        let bitmap_shared = make_shared_data(bitmap_shm_f, params.bitmap_size);
        let payload_shared = make_shared_data(payload_shm_f, params.payload_size);

        // dry_run
        run_qemu(&mut control);

        let aux_shm_f = OpenOptions::new()
            .read(true)
            .write(true)
            .open(&params.qemu_aux_buffer_filename)
            .expect("couldn't open aux buffer file");
        aux_shm_f.set_len(0x1000).unwrap();

        let aux_shm_f = OpenOptions::new()
            .write(true)
            .read(true)
            .open(&params.qemu_aux_buffer_filename)
            .expect("couldn't open aux buffer file");
        let mut aux_buffer = AuxBuffer::new(aux_shm_f);

        aux_buffer.validate_header();
        aux_buffer.config.protect_payload_buffer = 1;

        loop {
            if aux_buffer.result.hprintf == 1 {
                print!("HPRINTF {:?}\n", aux_buffer.misc);
            }

            if aux_buffer.result.state == 3 {
                break;
            }
            //println!("QEMU NOT READY");
            run_qemu(&mut control);
        }
        println!("QEMU READY");

        aux_buffer.config.reload_mode = 1;
        aux_buffer.config.timeout_sec = 0;
        aux_buffer.config.timeout_usec = 500_000;
        aux_buffer.config.changed = 1;

        //run_qemu(&mut control);
        //run_qemu(&mut control);

        return QemuProcess {
            process: child,
            aux: aux_buffer,
            ctrl: control,
            bitmap: bitmap_shared,
            payload: payload_shared,
            params,
        };
    }



    pub fn send_payload(&mut self) {
        let mut old_address: u64 = 0;
        loop {
            mem_barrier();
            run_qemu(&mut self.ctrl);
            mem_barrier();

            if self.aux.result.hprintf != 0 {
                print!("HPRINTF {:?}", self.aux.misc);
                continue;
            }

            if self.aux.result.success != 0 || self.aux.result.crash_found != 0 || self.aux.result.asan_found != 0 || self.aux.result.payload_write_attempt_found != 0 {
                break;
            }

            if self.aux.result.page_not_found != 0 {
                if old_address == self.aux.result.page_not_found_addr {
                    break;
                }
                old_address = self.aux.result.page_not_found_addr;
                self.aux.config.page_addr = self.aux.result.page_not_found_addr;
                self.aux.config.page_dump_mode = 1;
                self.aux.config.changed = 1;
            } 
            //else {
            //    break;
            //}
        }
    }

    pub fn wait(&mut self) {
        self.process.wait().unwrap();
    }

    pub fn shutdown(&mut self) {
        println!("Let's kill QEMU!");
        self.process.kill().unwrap();
        self.wait();
    }

    pub fn prepare_workdir(workdir: &str, target_path: Option<&str>) {
        Self::clear_workdir(workdir);
        let folders = vec![
            "/corpus/regular",
            "/metadata",
            "/corpus/crash",
            "/corpus/kasan",
            "/corpus/timeout",
            "/bitmaps",
            "/imports",
            "/snapshot",
            "/forced_imports",
        ];

        for folder in folders.iter() {
            fs::create_dir_all(format!("{}/{}", workdir, folder))
                .expect("couldn't initialize workdir");
        }
        OpenOptions::new()
            .create(true)
            .write(true)
            .open(format!("{}/filter", workdir))
            .unwrap();
        OpenOptions::new()
            .create(true)
            .write(true)
            .open(format!("{}/page_cache.lock", workdir))
            .unwrap();
        OpenOptions::new()
            .create(true)
            .write(true)
            .open(format!("{}/page_cache.dump", workdir))
            .unwrap();
        OpenOptions::new()
            .create(true)
            .write(true)
            .open(format!("{}/page_cache.addr", workdir))
            .unwrap();

        if let Some(target) = target_path {
            assert!(fs::metadata(target).unwrap().len() <= (128 << 20)); //limit of qemu_pt
            fs::copy(target, format!("{}/program", workdir)).unwrap();
        }else{
            OpenOptions::new().create(true).write(true).open(format!("{}/program", workdir)).unwrap();
        }
    }

    fn prepare_redqueen_workdir(workdir: &str, qemu_id: usize) {
        fs::create_dir_all(format!("{}/redqueen_workdir_{}", workdir, qemu_id))
            .expect("couldn't initialize workdir");
    }

    fn clear_workdir(workdir: &str) {
        let _ = fs::remove_dir_all(workdir);

        let project_name = Path::new(workdir)
            .file_name()
            .expect("Couldn't get project name from workdir!")
            .to_str()
            .expect("invalid chars in workdir path")
            .to_string();

        for p in glob::glob(&format!("/dev/shm/kafl_{}_*", project_name)).expect("couldn't glob??")
        {
            fs::remove_file(p.expect("invalid path found")).unwrap();
        }
    }
}
