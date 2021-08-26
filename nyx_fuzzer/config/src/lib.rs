extern crate serde;
#[macro_use]
extern crate serde_derive;

use std::time::Duration;

#[derive(Clone, Serialize, Deserialize)]
pub struct QemuKernelConfig {
    pub qemu_binary: String,
    pub target_pack: String,
    pub kernel: String,
    pub ramfs: String,
    pub debug: bool,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum SnapshotPath {
    Reuse(String),
    Create(String),
    DefaultPath,
}

#[derive(Clone, Serialize, Deserialize)]
pub struct QemuSnapshotConfig {
    pub qemu_binary: String,
    pub hda: String,
    pub sharedir: String,
    pub presnapshot: String,
    pub snapshot_path: SnapshotPath,
    pub debug: bool,
}

#[derive(Clone, Serialize, Deserialize)]
pub struct ForkServerConfig {
    pub args: Vec<String>,
    pub hide_output: bool,
    pub input_size: usize,
    pub env: Vec<String>,
}

#[derive(Clone, Serialize, Deserialize)]
pub enum FuzzRunnerConfig {
    QemuKernel(QemuKernelConfig),
    QemuSnapshot(QemuSnapshotConfig),
    ForkServer(ForkServerConfig),
}

#[derive(Clone, Serialize, Deserialize)]
pub struct FuzzerConfig {
    pub spec_path: String,
    pub workdir_path: String,
    pub bitmap_size: usize,
    pub mem_limit: usize,
    pub time_limit: Duration,
    pub target_binary: Option<String>,
    pub threads: usize,
    pub thread_id: usize,
    pub cpu_pin_start_at: usize,
}

#[derive(Clone, Serialize, Deserialize)]
pub struct Config {
    pub runner: FuzzRunnerConfig,
    pub fuzz: FuzzerConfig,
}
