pub mod newtypes;

use nix::fcntl;
use nix::libc::{
    __errno_location, shmat, shmctl, shmget, strerror, IPC_CREAT, IPC_EXCL, IPC_PRIVATE, IPC_RMID,
};
use nix::sys::signal::{self, Signal};
use nix::sys::stat;
use nix::sys::wait::WaitStatus;
use nix::unistd;
use nix::unistd::Pid;
use nix::unistd::{fork, ForkResult};
use std::ffi::{CStr, CString};
use std::os::unix::io::AsRawFd;
use std::os::unix::io::RawFd;

use std::io::BufReader;
use std::ptr;

use timeout_readwrite::TimeoutReader;

use byteorder::{LittleEndian, ReadBytesExt};
use std::fs::File;
use std::os::unix::io::FromRawFd;

use crate::exitreason::ExitReason;
use newtypes::*;
use snafu::ResultExt;

extern crate config;
use crate::config::{ForkServerConfig, FuzzerConfig};

pub struct ForkServer {
    inp_file: File,
    ctl_in: File,
    shared_data: *mut [u8],
    input_data: Vec<u8>,
    input_size: usize,
    st_out: std::io::BufReader<TimeoutReader<File>>,
}

impl ForkServer {
    pub fn new(cfg: &ForkServerConfig, fuzz_cfg: &FuzzerConfig) -> Self {
        let inp_file = tempfile::NamedTempFile::new().expect("couldn't create temp file");
        let (inp_file, in_path) = inp_file
            .keep()
            .expect("couldn't persists temp file for input");
        let inp_file_path = in_path
            .to_str()
            .expect("temp path should be unicode!")
            .to_string();
        let args = cfg
            .args
            .iter()
            .map(|s| {
                if s == "@@" {
                    inp_file_path.clone()
                } else {
                    s.to_string()
                }
            })
            .collect::<Vec<_>>();
        let (ctl_out, ctl_in) = nix::unistd::pipe().expect("failed to create ctl_pipe");
        let (st_out, st_in) = nix::unistd::pipe().expect("failed to create st_pipe");
        let (shm_file, shared_data) = ForkServer::create_shm(fuzz_cfg.bitmap_size);

        match fork().expect("couldn't fork") {
            // Parent returns
            ForkResult::Parent { child: _, .. } => {
                unistd::close(ctl_out).expect("coulnd't close ctl_out");
                unistd::close(st_in).expect("coulnd't close st_out");
                let mut st_out = BufReader::new(TimeoutReader::new(
                    unsafe { File::from_raw_fd(st_out) },
                    fuzz_cfg.time_limit,
                ));
                st_out
                    .read_u32::<LittleEndian>()
                    .expect("couldn't read child hello");
                return Self {
                    inp_file: inp_file,
                    ctl_in: unsafe { File::from_raw_fd(ctl_in) },
                    shared_data: shared_data,
                    st_out,
                    input_data: vec![0; cfg.input_size],
                    input_size: 0,
                };
            }
            //Child does complex stuff
            ForkResult::Child => {
                let forkserver_fd = 198; // from AFL config.h
                unistd::dup2(ctl_out, forkserver_fd as RawFd)
                    .expect("couldn't dup2 ctl_our to FROKSRV_FD");
                unistd::dup2(st_in, (forkserver_fd + 1) as RawFd)
                    .expect("couldn't dup2 ctl_our to FROKSRV_FD+1");

                unistd::dup2(inp_file.as_raw_fd(), 0).expect("couldn't dup2 input file to stdin");
                unistd::close(inp_file.as_raw_fd()).expect("couldn't close input file");

                unistd::close(ctl_in).expect("couldn't close ctl_in");
                unistd::close(ctl_out).expect("couldn't close ctl_out");
                unistd::close(st_in).expect("couldn't close ctl_out");
                unistd::close(st_out).expect("couldn't close ctl_out");

                let path = CString::new(fuzz_cfg.target_binary.as_ref().expect("forkserver requires target path").to_string())
                    .expect("binary path must not contain zero");
                let args = args
                    .into_iter()
                    .map(|s| CString::new(s).expect("args must not contain zero"))
                    .collect::<Vec<_>>();

                let shm_id = CString::new(format!("__AFL_SHM_ID={}", shm_file)).unwrap();

                //Asan options: set asan SIG to 223 and disable leak detection
                let asan_settings =
                    CString::new("ASAN_OPTIONS=exitcode=223,abort_on_erro=true,detect_leaks=0")
                        .expect("RAND_2089158993");

                let mut env = vec![shm_id, asan_settings];
                env.extend(
                    cfg.env
                        .iter()
                        .map(|str| CString::new(str.to_string()).unwrap()),
                );

                if cfg.hide_output {
                    let null = fcntl::open("/dev/null", fcntl::OFlag::O_RDWR, stat::Mode::empty())
                        .expect("couldn't open /dev/null");
                    unistd::dup2(null, 1 as RawFd).expect("couldn't dup2 /dev/null to stdout");
                    unistd::dup2(null, 2 as RawFd).expect("couldn't dup2 /dev/null to stderr");
                    unistd::close(null).expect("couldn't close /dev/null");
                }

                let arg_ref = &args.iter().map(|x| x.as_c_str()).collect::<Vec<&CStr>>()[..];
                let env_ref = &env.iter().map(|x| x.as_c_str()).collect::<Vec<&CStr>>()[..];

                println!("EXECVE {:?} {:?} {:?}", fuzz_cfg.target_binary, arg_ref, env_ref);
                unistd::execve(&path, arg_ref, env_ref).expect("couldn't execve target");
                unreachable!();
            }
        }
    }

    pub fn run_data(&mut self, data: &[u8]) -> Result<ExitReason, SubprocessError> {
        self.input_data[0..data.len()].copy_from_slice(data);
        self.input_size = data.len();
        return self.run();
    }

    pub fn run(&mut self) -> Result<ExitReason, SubprocessError> {
        for i in self.get_bitmap_mut().iter_mut() {
            *i = 0;
        }
        unistd::ftruncate(self.inp_file.as_raw_fd(), 0).context(QemuRunNix {
            task: "Couldn't truncate inp_file",
        })?;
        unistd::lseek(self.inp_file.as_raw_fd(), 0, unistd::Whence::SeekSet).context(
            QemuRunNix {
                task: "Couldn't seek inp_file",
            },
        )?;
        unistd::write(
            self.inp_file.as_raw_fd(),
            &self.input_data[0..self.input_size],
        )
        .context(QemuRunNix {
            task: "Couldn't write data to inp_file",
        })?;
        unistd::lseek(self.inp_file.as_raw_fd(), 0, unistd::Whence::SeekSet).context(
            QemuRunNix {
                task: "Couldn't seek inp_file",
            },
        )?;

        unistd::write(self.ctl_in.as_raw_fd(), &[0, 0, 0, 0]).context(QemuRunNix {
            task: "Couldn't send start command",
        })?;

        let pid = Pid::from_raw(self.st_out.read_i32::<LittleEndian>().context(QemuRunIO {
            task: "Couldn't read target pid",
        })?);

        if let Ok(status) = self.st_out.read_i32::<LittleEndian>() {
            return Ok(ExitReason::from_wait_status(
                WaitStatus::from_raw(pid, status).expect("402104968"),
            ));
        }
        signal::kill(pid, Signal::SIGKILL).context(QemuRunNix {
            task: "Couldn't kill timed out process",
        })?;
        self.st_out.read_u32::<LittleEndian>().context(QemuRunIO {
            task: "couldn't read timeout exitcode",
        })?;
        return Ok(ExitReason::Timeout);
    }

    pub fn get_bitmap_mut(&mut self) -> &mut [u8] {
        unsafe { return &mut *self.shared_data }
    }
    pub fn get_bitmap(&self) -> &[u8] {
        unsafe { return &*self.shared_data }
    }

    pub fn get_input_mut(&mut self) -> &mut [u8] {
        return &mut self.input_data[..];
    }

    pub fn get_input(&self) -> &[u8] {
        return &self.input_data[..];
    }

    pub fn set_input_size(&mut self, size: usize) {
        assert!(size <= self.input_data.len());
        self.input_size = size;
    }

    fn create_shm(bitmap_size: usize) -> (i32, *mut [u8]) {
        unsafe {
            let shm_id = shmget(IPC_PRIVATE, bitmap_size, IPC_CREAT | IPC_EXCL | 0o600);
            if shm_id < 0 {
                panic!(
                    "shm_id {:?}",
                    CString::from_raw(strerror(*__errno_location()))
                );
            }

            let trace_bits = shmat(shm_id, ptr::null(), 0);
            if (trace_bits as isize) < 0 {
                panic!(
                    "shmat {:?}",
                    CString::from_raw(strerror(*__errno_location()))
                );
            }

            let res = shmctl(shm_id, IPC_RMID, 0 as *mut nix::libc::shmid_ds);
            if res < 0 {
                panic!(
                    "shmclt {:?}",
                    CString::from_raw(strerror(*__errno_location()))
                );
            }
            return (shm_id, trace_bits as *mut [u8; 1 << 16]);
        }
    }
}