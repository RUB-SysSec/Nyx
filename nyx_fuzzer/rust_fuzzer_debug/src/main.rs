extern crate config;
extern crate fuzz_runner;
extern crate helpers;
extern crate structured_fuzzer;
extern crate core_affinity;

use std::time::Duration;

use fuzz_runner::nyx::qemu_process_new_from_snapshot;

use std::fs::File;
use std::io::Read;
use std::fs;

use config::{
    ForkServerConfig, FuzzRunnerConfig, FuzzerConfig, QemuKernelConfig, QemuSnapshotConfig,
};

fn main() {
    println!("Hello, world!");

    let snap_cfg = QemuSnapshotConfig {
        qemu_binary: "/home/sschumilo/kafl/QEMU-PT/x86_64-softmmu/qemu-system-x86_64".to_string(),
        //hda: "/home/sschumilo/VMs/ubuntu_18_04_4_asan.qcow2".to_string(),
        //presnapshot: "/home/sschumilo/VMs/ubuntu_18_04_4_asan/ubuntu_snapshot_asan".to_string(),
        //sharedir: "/home/sschumilo/VMs/ubuntu_18_04_4_asan/ubuntu_qemu_sharedir".to_string(),
        hda: "/home/sschumilo/VMs/freebsd_11_3_asan/freebsd_11_3_asan.qcow2".to_string(),
        presnapshot: "/home/sschumilo/VMs/freebsd_11_3_asan/freebsd_11_3_asan_pre_snapshot/".to_string(),
        sharedir: "/home/sschumilo/VMs/freebsd_11_3_asan/freebsd_11_3_asan_sharedir/".to_string(),
        //ram_size: 1024,
        debug: true,
    };

    let config = FuzzerConfig {
        target_binary: None,
        bitmap_size: 1 << 16,
        time_limit: Duration::from_millis(10000),
        spec_path: "../hypertrash_spec/hexaschrott_freebsd.msgp".to_string(),
        workdir_path: "/tmp/fuzz_struct_test_debug".to_string(),
        mem_limit: 1024, //MB
        threads: 1,
        thread_id: 0,
    };


    let runner_cfg = FuzzRunnerConfig::QemuSnapshot(snap_cfg);

    match runner_cfg.clone() {
        FuzzRunnerConfig::QemuSnapshot(cfg) => {
            let mut runner = qemu_process_new_from_snapshot(&cfg, &config);

            runner.aux.config.timeout_sec = 2;
            runner.aux.config.timeout_usec = 500_000;

            let paths = fs::read_dir("/tmp/fuzz_struct_test_bk2/corpus/timeout/").unwrap();

            for path in paths {
                let final_path = path.unwrap().path();
                let path_str = final_path.to_str().unwrap();
                if path_str.ends_with("bin"){
                    println!("path: {:?}", final_path);
                    let mut f = File::open(final_path).expect("no file found");
                    f.read(runner.payload).expect("buffer overflow");

                    runner.send_payload();

                
                    println!("AUX -> {:?}", runner.aux.result);
                    if runner.aux.result.timeout_found == 1{
                        println!(" ---> TIMEOUT YO");
                    }
                }
            }

            

            


            //let input_path = "/dev/shm/input_data";
            //let input_mmap = helpers::make_shared_data_from_path(input_path, runner.input_size);
            //let srv = ForkServer::new(&cfg, &fuzz_cfg);

        }
        _ => unreachable!(),
    }
}
