pub mod aux_buffer;
pub mod mem_barrier;
pub mod params;
pub mod qemu_process;

pub use qemu_process::QemuProcess;

extern crate config;
use crate::config::{QemuKernelConfig, QemuSnapshotConfig, FuzzerConfig, SnapshotPath};

pub fn qemu_process_new_from_kernel(cfg: &QemuKernelConfig, fuzz_cfg: &FuzzerConfig) -> qemu_process::QemuProcess {
    let params = params::KernelVmParams {
        qemu_binary: cfg.qemu_binary.to_string(),
        kernel: cfg.kernel.to_string(),
        ramfs: cfg.ramfs.to_string(),
        ram_size: fuzz_cfg.mem_limit,
        bitmap_size: fuzz_cfg.bitmap_size,
        debug: cfg.debug,
    };
    let qemu_id = 1;
    let qemu_params = params::QemuParams::new_from_kernel(&fuzz_cfg.workdir_path, qemu_id, &params);

    qemu_process::QemuProcess::prepare_workdir(&fuzz_cfg.workdir_path, Some(&cfg.target_pack));

    return qemu_process::QemuProcess::new(qemu_params);
}

pub fn qemu_process_new_from_snapshot(cfg: &QemuSnapshotConfig,  fuzz_cfg: &FuzzerConfig) -> qemu_process::QemuProcess {

    let snapshot_path = match &cfg.snapshot_path{
        SnapshotPath::Create(_x) => panic!(),
        SnapshotPath::Reuse(x) => SnapshotPath::Reuse(x.to_string()),
        SnapshotPath::DefaultPath => {
            if fuzz_cfg.thread_id == 0 {
                SnapshotPath::Create(format!("{}/snapshot/",fuzz_cfg.workdir_path))
            } else {
                SnapshotPath::Reuse(format!("{}/snapshot/",fuzz_cfg.workdir_path))
            }
        }
    };

    let params = params::SnapshotVmParams {
        qemu_binary: cfg.qemu_binary.to_string(),
        hda: cfg.hda.to_string(),
        sharedir: cfg.sharedir.to_string(),
        presnapshot: cfg.presnapshot.to_string(),
        ram_size: fuzz_cfg.mem_limit,
        bitmap_size: fuzz_cfg.bitmap_size,
        debug: cfg.debug,
        snapshot_path,
    };
    let qemu_id = fuzz_cfg.thread_id;
    let qemu_params = params::QemuParams::new_from_snapshot(&fuzz_cfg.workdir_path, qemu_id, fuzz_cfg.cpu_pin_start_at, &params);
    if qemu_id == 0{
        qemu_process::QemuProcess::prepare_workdir(&fuzz_cfg.workdir_path, None);
    }

    return qemu_process::QemuProcess::new(qemu_params);
}


#[cfg(test)]
mod tests {
    //use crate::aux_buffer::*;
    use super::params::*;
    use super::qemu_process::*;
    //use std::{thread, time};

    #[test]
    fn it_works() {
        let workdir = "/tmp/workdir_test";
        let params = KernelVmParams {
            qemu_binary: "/home/kafl/NEW2/QEMU-PT_4.2.0/x86_64-softmmu/qemu-system-x86_64"
                .to_string(),
            kernel: "/home/kafl/Target-Components/linux_initramfs/bzImage-linux-4.15-rc7"
                .to_string(),
            ramfs: "/home/kafl/Target-Components/linux_initramfs/init.cpio.gz".to_string(),
            ram_size: 1000,
            bitmap_size: 0x1 << 16,
            debug: false,
        };
        let qemu_id = 1;
        let qemu_params = QemuParams::new_from_kernel(workdir, qemu_id, &params);

        QemuProcess::prepare_workdir(&workdir, Some("/tmp/zsh_fuzz"));

        let mut qemu_process = QemuProcess::new(qemu_params);

        for _i in 0..100 {
            qemu_process.send_payload();
        }
        println!("test done");
    }
}
