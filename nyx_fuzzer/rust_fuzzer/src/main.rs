extern crate config;
extern crate core_affinity;
extern crate fuzz_runner;
extern crate helpers;
extern crate serde;
extern crate structured_fuzzer;
#[macro_use]
extern crate serde_derive;
extern crate rmp_serde;
extern crate ron;
extern crate rand;

use clap::{value_t, App, Arg};

use crate::queue::Queue;

use std::fs::File;
use std::thread;
use std::time::Duration;

mod bitmap;
mod fuzzer;
mod input;
mod queue;
mod romu;

use rand::thread_rng;
use crate::rand::Rng;

use fuzzer::{StructFuzzer, StructuredForkServer};

use structured_fuzzer::graph_mutator::spec_loader;
use crate::romu::*;
use fuzz_runner::nyx::qemu_process_new_from_kernel;
use fuzz_runner::nyx::qemu_process_new_from_snapshot;

use config::{Config, FuzzRunnerConfig};

fn main() {
    let matches = App::new("nyx")
        .about("Fuzz EVERYTHING!")
        .arg(
            Arg::with_name("config")
                .short("c")
                .long("config")
                .value_name("CONFIG_PATH")
                .takes_value(true)
                .help("path to the config.ron, default: ./config.ron"),
        )
        .arg(
            Arg::with_name("workdir")
                .short("w")
                .long("workdir")
                .value_name("WORKDIR_PATH")
                .takes_value(true)
                .help("overrides the workdir path in the config"),
        )
        .arg(
            Arg::with_name("cpu_start")
                .short("p")
                .long("cpu")
                .value_name("CPU_START")
                .takes_value(true)
                .help("overrides the config value for the first CPU to pin threads to"),
        )
        .arg(
            Arg::with_name("seed")
                .long("seed")
                .value_name("SEED")
                .takes_value(true)
                .help("runs the fuzzer with a specific seed, if not give, a seed is generated from a secure prng"),
        )
        .get_matches();

    let config = matches
        .value_of("config")
        .unwrap_or("./config.ron")
        .to_string();

    let cfg_file = File::open(&config).unwrap();
    let cfg: Config = ron::de::from_reader(cfg_file).unwrap();

    let mut config = cfg.fuzz;
    let runner_cfg = cfg.runner;

    if let Some(path) = matches.value_of("workdir") {
        config.workdir_path = path.to_string();
    }
    if let Ok(start_cpu_id) = value_t!(matches, "cpu_start", usize) {
        config.cpu_pin_start_at = start_cpu_id;
    }

    let file = File::open(&config.spec_path).expect(&format!(
        "couldn't open spec (File not found: {}",
        config.spec_path
    ));
    let spec = spec_loader::load_spec_from_read(file);
    let queue = Queue::new(&config);

    let mut thread_handles = vec![];
    let core_ids = core_affinity::get_core_ids().unwrap();
    let seed = value_t!(matches, "cpu_start", u64).unwrap_or(thread_rng().gen());
    let mut rng = RomuPrng::new_from_u64(seed);

    for i in 0..config.threads {
        let mut cfg = config.clone();
        cfg.thread_id = i;

        let spec1 = spec.clone();
        let queue1 = queue.clone();
        let core_id = core_ids[(i + cfg.cpu_pin_start_at) % core_ids.len()].clone();
        let thread_seed = rng.next_u64();
        match runner_cfg.clone() {
            FuzzRunnerConfig::QemuSnapshot(run_cfg) => {
                thread_handles.push(thread::spawn(move || {
                    core_affinity::set_for_current(core_id);
                    let runner = qemu_process_new_from_snapshot(&run_cfg, &cfg);
                    let mut fuzzer = StructFuzzer::new(runner, cfg, spec1, queue1,thread_seed);
                    fuzzer.run();
                }));
                if i == 0 {
                    std::thread::sleep(Duration::from_secs(1));
                }
            }
            _ => unreachable!(), //FuzzRunnerConfig::QemuKernel(ref run_cfg) => {
                                 //    let runner = qemu_process_new_from_kernel(&run_cfg, &cfg);
                                 //    let mut fuzzer = StructFuzzer::new(runner, cfg, spec.clone(), queue.clone());
                                 //    fuzzer.run();
                                 //}
                                 //FuzzRunnerConfig::ForkServer(ref run_cfg) => {
                                 //    let runner = StructuredForkServer::new(run_cfg, &cfg);
                                 //    let mut fuzzer = StructFuzzer::new(runner, cfg, spec.clone(), queue.clone());
                                 //    fuzzer.run();
                                 //}
        }
    }
    for t in thread_handles.into_iter() {
        t.join().unwrap();
    }
}
