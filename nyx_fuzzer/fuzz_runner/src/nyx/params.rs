use std::path::Path;
use crate::config::SnapshotPath;

pub struct KernelVmParams {
    pub qemu_binary: String,
    pub kernel: String,
    pub ramfs: String,
    pub ram_size: usize,
    pub bitmap_size: usize,
    pub debug: bool,
}

pub struct SnapshotVmParams{
    pub qemu_binary: String,
    pub hda: String,
    pub sharedir: String,
    pub presnapshot: String,
    pub snapshot_path: SnapshotPath,
    pub ram_size: usize,
    pub bitmap_size: usize,
    pub debug: bool,
}

pub struct QemuParams {
    pub cmd: Vec<String>,
    pub qemu_aux_buffer_filename: String,
    pub control_filename: String,
    pub bitmap_filename: String,
    pub payload_filename: String,
    pub binary_filename: String,
    pub workdir: String,
    pub qemu_id: usize,
    pub bitmap_size: usize,
    pub payload_size: usize,
}

impl QemuParams {
     pub fn new_from_snapshot(workdir: &str, qemu_id: usize, cpu: usize, params: &SnapshotVmParams) -> QemuParams{
        let project_name = Path::new(workdir)
        .file_name()
        .expect("Couldn't get project name from workdir!")
        .to_str()
        .expect("invalid chars in workdir path")
        .to_string();

    let qemu_aux_buffer_filename = format!("{}/aux_buffer_{}", workdir, qemu_id);
    let payload_filename = format!("/dev/shm/kafl_{}_qemu_payload_{}", project_name, qemu_id);
    //let tracedump_filename = format!("/dev/shm/kafl_{}_pt_trace_dump_{}", project_name, qemu_id);
    let binary_filename = format!("{}/program", workdir);
    let bitmap_filename = format!("/dev/shm/kafl_{}_bitmap_{}", project_name, qemu_id);
    let control_filename = format!("{}/interface_{}", workdir, qemu_id);

    let mut cmd = vec![];
    cmd.push(params.qemu_binary.to_string());
    cmd.push("-hda".to_string());
    cmd.push(params.hda.to_string());

    if !params.debug {
        cmd.push("-display".to_string());
        cmd.push("none".to_string());
    } else {
        cmd.push("-vnc".to_string());
        cmd.push(format!(":{}",qemu_id+cpu));
    }

    cmd.push("-serial".to_string());
    cmd.push("mon:stdio".to_string());

    cmd.push("-enable-kvm".to_string());

    cmd.push("-net".to_string());
    cmd.push("none".to_string());

    cmd.push("-k".to_string());
    cmd.push("de".to_string());

    cmd.push("-m".to_string());
    cmd.push(params.ram_size.to_string());

    cmd.push("-chardev".to_string());
    cmd.push(format!(
        "socket,server,path={},id=kafl_interface",
        control_filename
    ));

     // -fast_vm_reload path=/tmp/fuzz_workdir/snapshot/,load=off,pre_path=/home/kafl/ubuntu_snapshot
  
    cmd.push("-device".to_string());
    let mut nyx_ops = format!("kafl,chardev=kafl_interface");
    nyx_ops += &format!(",bitmap_size={}", params.bitmap_size);
    nyx_ops += &format!(",worker_id={}", qemu_id);
    nyx_ops += &format!(",workdir={}", workdir);
    nyx_ops += &format!(",sharedir={}", params.sharedir);
    //nyx_ops += &format!(",ip0_a=0x1000,ip0_b=0x7ffffffff000");
    //nyx_ops += &format!(",ip0_a=ffff800000000000,ip0_b=ffffffffffffffff");

    cmd.push(nyx_ops);

    cmd.push("-machine".to_string());
    cmd.push("kAFL64-v1".to_string());

    cmd.push("-cpu".to_string());
    cmd.push("kAFL64-Hypervisor-v1".to_string());
    //cmd.push("kvm64-v1,".to_string());

    match &params.snapshot_path {
        SnapshotPath::Create(path) => {
            cmd.push("-fast_vm_reload".to_string());
            cmd.push(format!("path={},load=off,pre_path={}", path,params.presnapshot));
        },
        SnapshotPath::Reuse(path) => {
            cmd.push("-fast_vm_reload".to_string());
            cmd.push(format!("path={},load=on", path));
        }
        SnapshotPath::DefaultPath => panic!(),
    }
    
    /*
    bin = read_binary_file("/tmp/zsh_fuzz")
    assert(len(bin)<= (128 << 20))
    atomic_write(binary_filename, bin)
    */
    return QemuParams {
        cmd,
        qemu_aux_buffer_filename,
        control_filename,
        bitmap_filename,
        payload_filename,
        binary_filename,
        workdir: workdir.to_string(),
        qemu_id,
        bitmap_size: params.bitmap_size,
        payload_size: (1 << 16),
    };
    }

    pub fn new_from_kernel(workdir: &str, qemu_id: usize, params: &KernelVmParams) -> QemuParams {
        //prepare_working_dir(workdir)

        let project_name = Path::new(workdir)
            .file_name()
            .expect("Couldn't get project name from workdir!")
            .to_str()
            .expect("invalid chars in workdir path")
            .to_string();

        let qemu_aux_buffer_filename = format!("{}/aux_buffer_{}", workdir, qemu_id);
        let payload_filename = format!("/dev/shm/kafl_{}_qemu_payload_{}", project_name, qemu_id);
        //let tracedump_filename = format!("/dev/shm/kafl_{}_pt_trace_dump_{}", project_name, qemu_id);
        let binary_filename = format!("{}/program", workdir);
        let bitmap_filename = format!("/dev/shm/kafl_{}_bitmap_{}", project_name, qemu_id);
        let control_filename = format!("{}/interface_{}", workdir, qemu_id);

        let mut cmd = vec![];
        cmd.push(params.qemu_binary.to_string());
        cmd.push("-kernel".to_string());
        cmd.push(params.kernel.to_string());

        cmd.push("-initrd".to_string());
        cmd.push(params.ramfs.to_string());

        cmd.push("-append".to_string());
        cmd.push("nokaslr oops=panic nopti".to_string());

        if !params.debug {
            cmd.push("-display".to_string());
            cmd.push("none".to_string());
        }

        cmd.push("-serial".to_string());
        cmd.push("mon:stdio".to_string());

        cmd.push("-enable-kvm".to_string());

        cmd.push("-net".to_string());
        cmd.push("none".to_string());

        cmd.push("-k".to_string());
        cmd.push("de".to_string());

        cmd.push("-m".to_string());
        cmd.push(params.ram_size.to_string());

        cmd.push("-cdrom".to_string());
        cmd.push("/home/kafl/rust_dev/nyx/syzkaller_spec/cd.iso".to_string());

        cmd.push("-chardev".to_string());
        cmd.push(format!(
            "socket,server,path={},id=kafl_interface",
            control_filename
        ));

        cmd.push("-device".to_string());
        let mut nyx_ops = format!("kafl,chardev=kafl_interface");
        nyx_ops += &format!(",bitmap_size={}", params.bitmap_size);
        nyx_ops += &format!(",worker_id={}", qemu_id);
        nyx_ops += &format!(",workdir={}", workdir);
        //nyx_ops += &format!(",ip0_a=0x1000,ip0_b=0x7ffffffff000");
        nyx_ops += &format!(",ip0_a=ffff800000000000,ip0_b=ffffffffffffffff");

        cmd.push(nyx_ops);

        cmd.push("-cpu".to_string());
        //cmd.push("kAFL64-Hypervisor-v1,+vmx".to_string());
        cmd.push("kvm64-v1,+vmx".to_string());

        cmd.push("-fast_vm_reload".to_string());
        cmd.push(format!("path={}/snapshot/,load=off", workdir));

        /*
        bin = read_binary_file("/tmp/zsh_fuzz")
        assert(len(bin)<= (128 << 20))
        atomic_write(binary_filename, bin)
        */
        return QemuParams {
            cmd,
            qemu_aux_buffer_filename,
            control_filename,
            bitmap_filename,
            payload_filename,
            binary_filename,
            workdir: workdir.to_string(),
            qemu_id,
            bitmap_size: params.bitmap_size,
            payload_size: (128 << 10),
        };
    }
}
