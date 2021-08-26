use crate::bitmap::StorageReason;
use crate::bitmap::{Bitmap, BitmapHandler};
use crate::fuzz_runner::FuzzRunner;
use crate::fuzz_runner::{CFGInfo, ExitReason, ForkServer, RedqueenInfo, TestInfo};
use crate::fuzz_runner::{RedqueenEvent,RedqueenBPType};
use crate::helpers;
use crate::input::{Input, InputID, InputState};
use crate::queue::Queue;
use crate::romu::*;
use crate::structured_fuzzer::custom_dict::{CustomDict,DictEntry};
use crate::structured_fuzzer::graph_mutator::graph_storage::{RefGraph, VecGraph};
use crate::structured_fuzzer::graph_mutator::spec::GraphSpec;
use crate::structured_fuzzer::mutator::MutationStrategy;
use crate::structured_fuzzer::mutator::Mutator;
use crate::structured_fuzzer::random::distributions::Distributions;
use crate::structured_fuzzer::GraphStorage;

use crate::config::{ForkServerConfig, FuzzerConfig};

use std::collections::{HashMap,HashSet};
use std::error::Error;
use std::rc::Rc;

pub struct StructuredForkServer {
    srv: ForkServer,
    input_mmap: &'static mut [u8],
}

impl StructuredForkServer {
    pub fn new(cfg: &ForkServerConfig, fuzz_cfg: &FuzzerConfig) -> Self {
        let input_path = "/dev/shm/input_data";
        let mut cfg = (*cfg).clone();
        cfg.env.push(format!("STRUCT_INPUT_PATH={}", input_path));
        let input_mmap = helpers::make_shared_data_from_path(input_path, cfg.input_size);
        let srv = ForkServer::new(&cfg, &fuzz_cfg);

        return Self { srv, input_mmap };
    }
}

impl FuzzRunner for StructuredForkServer {
    fn run_test(&mut self) -> Result<TestInfo, Box<dyn Error>> {
        self.srv.run_test()
    }

    fn run_redqueen(&mut self) -> Result<RedqueenInfo, Box<dyn Error>> {
        self.srv.run_redqueen()
    }
    fn run_cfg(&mut self) -> Result<CFGInfo, Box<dyn Error>> {
        self.srv.run_cfg()
    }
    fn shutdown(self) -> Result<(), Box<dyn Error>> {
        self.srv.shutdown()
    }
    fn input_buffer(&mut self) -> &mut [u8] {
        return &mut self.input_mmap;
    }
    fn bitmap_buffer(&self) -> &[u8] {
        self.srv.bitmap_buffer()
    }
    fn set_input_size(&mut self, _size: usize) {}
}

pub trait GetStructStorage {
    fn get_struct_storage(&mut self, checksum: u64) -> RefGraph;
}

impl<T: FuzzRunner> GetStructStorage for T {
    fn get_struct_storage(&mut self, checksum: u64) -> RefGraph {
        return RefGraph::new_from_slice(self.input_buffer(), checksum);
    }
}

pub struct StructFuzzer<Fuzz: FuzzRunner + GetStructStorage> {
    fuzzer: Fuzz,
    queue: Queue,
    master_rng: RomuPrng,
    rng: Distributions,
    mutator: Mutator,
    bitmaps: BitmapHandler,
    config: FuzzerConfig,
}

impl<Fuzz: FuzzRunner + GetStructStorage> StructFuzzer<Fuzz> {
    pub fn new(fuzzer: Fuzz, config: FuzzerConfig, spec: GraphSpec, queue: Queue, seed: u64) -> Self {
        let rng = Distributions::new();

        let mutator = Mutator::new(spec);

        let bitmaps = BitmapHandler::new(fuzzer.bitmap_buffer().len());
        let master_rng = RomuPrng::new_from_u64(seed);

        return Self {
            fuzzer,
            queue,
            master_rng,
            rng,
            mutator,
            bitmaps,
            config,
        };
    }

    fn perform_run<F>(&mut self, f: F) -> Option<(TestInfo, MutationStrategy)>
    where
        F: Fn(&mut Mutator, &Queue, &Distributions, &mut RefGraph) -> MutationStrategy,
    {
        let (seed_x, seed_y) = (self.master_rng.next_u64(), self.master_rng.next_u64());
        self.rng.set_full_seed(seed_x, seed_y);
        let strategy = {
            let mut storage = self.fuzzer.get_struct_storage(self.mutator.spec.checksum);
            f(&mut self.mutator, &self.queue, &self.rng, &mut storage)
        };
        //println!("EXEC.....");
        let res = self.fuzzer.run_test();

        if let Ok(exec_res) = res {
            if let Some(_) = self
                .bitmaps
                .check_new_bytes(self.fuzzer.bitmap_buffer(), &exec_res.exitreason)
            {
                if let Some(new_bytes) = self
                    .queue
                    .check_new_bytes(self.fuzzer.bitmap_buffer(), &exec_res.exitreason)
                {
                    let data = {
                        let mut storage =
                            self.fuzzer.get_struct_storage(self.mutator.spec.checksum);
                        //let strategy = f(&self.mutator, &self.queue, &self.rng, &mut storage);
                        self.mutator.dump_graph(&storage)
                    };

                    let mut input = Input::new(
                        data,
                        strategy,
                        new_bytes,
                        Bitmap::new_from_buffer(self.fuzzer.bitmap_buffer()),
                        exec_res.exitreason.clone(),
                        std::time::Duration::from_millis(0),
                    );

                    self.filter_nondet_bytes(&mut input);

                    if input.storage_reasons.len() > 0 {
                        self.new_input(&input);
                        self.queue.add(input, &self.mutator.spec);
                    }
                }
            }
            return Some((exec_res, strategy));
        }
        return None;
    }

    fn new_input(&mut self, input: &Input) {
        use std::fs;
        println!(
            "Thread {} Found input {} (len:{}) {}/{} new bytes by {:?}",
            self.config.thread_id,
            input.exit_reason.name(),
            input.data.node_len(&self.mutator.spec),
            input.storage_reasons.iter().filter(|r| r.old == 0).count(),
            input.storage_reasons.len(),
            input.found_by
        );

        fs::create_dir_all(&format!(
            "{}/corpus/{}",
            self.config.workdir_path,
            input.exit_reason.name()
        ))
        .unwrap();

        if !self.queue.should_minimize(&input) {
            //println!(
            //    "writing to {}",
            //    &format!(
            //        "{}/corpus/{}/cnt_{}.py",
            //        self.config.workdir_path,
            //        input.exit_reason.name(),
            //        self.others
            //    )
            //);
            let id = self.queue.next_id();
            input.data.write_to_file(
                &format!(
                    "{}/corpus/{}/cnt_{}.bin",
                    self.config.workdir_path,
                    input.exit_reason.name(),
                    id
                ),
                &self.mutator.spec,
            );
            input.data.write_to_script_file(
                &format!(
                    "{}/corpus/{}/cnt_{}.py",
                    self.config.workdir_path,
                    input.exit_reason.name(),
                    id
                ),
                &self.mutator.spec,
            );
            match &input.exit_reason {
                ExitReason::Crash(desc) => {
                    std::fs::write(
                        &format!(
                            "{}/corpus//{}/{}.log",
                            self.config.workdir_path,
                            input.exit_reason.name(),
                            id,
                        ),
                        desc,
                    )
                    .unwrap();
                }
                ExitReason::InvalidWriteToPayload(desc) => {
                    std::fs::write(
                        &format!(
                            "{}/corpus//{}/{}.log",
                            self.config.workdir_path,
                            input.exit_reason.name(),
                            id
                        ),
                        desc,
                    )
                    .unwrap();
                }
                _ => {}
            }
        }
    }

    fn perform_gen(&mut self) {
        self.perform_run(|mutator, _queue, rng, storage| {
            mutator.generate(100, storage, &rng);
            MutationStrategy::Generate
        });
    }

    fn perform_min(&mut self, input: &Input) {
        //println!(
        //    "Thread {} performs min on {:?}",
        //    self.config.thread_id, input.id
        //);
        self.queue.set_state(input, InputState::Havoc);

        let num_new_bytes = input.storage_reasons.iter().filter(|r| r.old == 0).count();
        let num_new_bits = input.storage_reasons.len();
        let mut num_splits = 0;

        if num_new_bytes > 0 {
            //println!(
            //    "splitting: new bytes: {:?}",
            //    input
            //        .storage_reasons
            //        .iter()
            //        .filter(|r| r.old == 0)
            //        .map(|r| r.index)
            //        .collect::<Vec<_>>()
            //);
            let inputs = self.minimize_new_bytes(&input);
            num_splits = inputs.len();
            for inp in inputs.into_iter().map(|inp| inp.into_rc()) {
                //println!(
                //    "new bytes for input (size {} ) {:?}",
                //    inp.data.node_len(&self.mutator.spec),
                //    inp.storage_reasons
                //        .iter()
                //        .filter(|r| r.old == 0)
                //        .map(|r| r.index)
                //        .collect::<Vec<_>>()
                //);
                let id;
                if let Ok(input) = Rc::try_unwrap(inp) {
                    id = self.queue.add(input, &self.mutator.spec);
                } else {
                    unreachable!();
                }
                if let Some(id) = id {
                    let inp = self.queue.get_input(id);
                    let data = &inp.read().unwrap().data;
                    self.get_trace(&data);
                    data.write_to_file(
                        &format!(
                            "{}/corpus/normal/cov_{}.bin",
                            self.config.workdir_path,
                            id.as_usize(),
                        ),
                        &self.mutator.spec,
                    );

                    data.write_to_script_file(
                        &format!(
                            "{}/corpus/normal/cov_{}.py",
                            self.config.workdir_path,
                            id.as_usize(),
                        ),
                        &self.mutator.spec,
                    );
                    std::fs::copy(
                        &format!(
                            "{}/redqueen_workdir_{}/pt_trace_results.txt",
                            self.config.workdir_path,
                            self.config.thread_id
                        ),
                        &format!(
                            "{}/corpus/normal/cov_{}.trace",
                            self.config.workdir_path,
                            id.as_usize(),
                        ),
                    )
                    .unwrap();

                    let info = self.get_rq_trace(&data);
                    let dict = Self::custom_dict_from_rq_data(&info.bps);
                    self.queue.set_custom_dict(&input, dict);
                    std::fs::copy(
                        &format!(
                            "{}/redqueen_workdir_{}/redqueen_results.txt",
                            self.config.workdir_path,
                            self.config.thread_id
                        ),
                        &format!(
                            "{}/corpus/normal/cov_{}.rq",
                            self.config.workdir_path,
                            id.as_usize(),
                        ),
                    )
                    .unwrap();

                }
            }
        }
        //if num_new_bits > 0 || num_splits > 1 {
        //    let min_input = self.minimize_new_bits(&input);
        //    self.queue.add(Rc::new(min_input), &self.mutator.spec);
        //}
    }


    pub fn custom_dict_from_rq_data(data: &[RedqueenEvent]) -> CustomDict{
        let mut groups = HashMap::new();
        for ev in data.iter(){
            let group = groups.entry(ev.addr).or_insert_with(|| vec!());
            match ev.bp_type{
             RedqueenBPType::Cmp | RedqueenBPType::Sub =>{
                 group.push(DictEntry::Replace(ev.lhs.clone(), ev.rhs.clone()));
                 if !ev.imm {
                    group.push(DictEntry::Replace(ev.rhs.clone(), ev.lhs.clone()));
                 }
            },
             RedqueenBPType::Str => {},
            }
        }
        let groups = groups.into_iter().map(|(k,v)| v).filter(|v| v.len() > 0 ).collect::<Vec<_>>();
        return CustomDict::new_from_groups(groups);
    }

    fn perform_havoc(&mut self, inp: &Input) {
        //println!(
        //    "Thread {} performs havoc on {:?}",
        //    self.config.thread_id, inp.id
        //);
        self.perform_run(|mutator, queue, rng, storage| {
            mutator.mutate(&inp.data, &inp.custom_dict, queue, storage, rng)
        });
    }

    pub fn iter(&mut self) {
        if self.queue.len() == 0 {
            self.perform_gen();
        } else {
            let entry = self.queue.schedule(&mut self.rng).read().unwrap().clone();
            match entry.state {
                InputState::Minimize => self.perform_min(&entry),
                InputState::Havoc => {
                    for _ in 0..10 {
                        self.perform_havoc(&entry);
                    }
                }
            }
        }
    }

    fn filter_nondet_bytes(&mut self, input: &mut Input) {
        let mut res = input.storage_reasons.clone();
        let mut bits = vec![];
        for _i in 0..10 {
            self.fuzzer.run_test().unwrap();
            let bitmap = self.fuzzer.bitmap_buffer();
            bits.push(bitmap.iter().filter(|e| **e > 0).count());
            res = res
                .into_iter()
                .filter(|reason| bitmap[reason.index] >= reason.new)
                .collect::<Vec<_>>();
        }
        //println!("nondet pass: {:?}", bits);
        input.storage_reasons = res;
    }

    fn get_trace(&mut self, graph: &VecGraph) {
        let mut storage = self.fuzzer.get_struct_storage(self.mutator.spec.checksum);
        storage.copy_from(graph);
        self.fuzzer.run_cfg().unwrap();
    }

    fn get_rq_trace(&mut self, graph: &VecGraph) -> RedqueenInfo {
        let mut storage = self.fuzzer.get_struct_storage(self.mutator.spec.checksum);
        storage.copy_from(graph);
        return self.fuzzer.run_redqueen().unwrap();
    }

    fn update_min_graphs(
        &mut self,
        new_bytes: &Vec<StorageReason>,
        exec_res: TestInfo,
        graphs: &mut Vec<Rc<Input>>,
    ) -> bool {
        let mut res = false;
        let dumped_graph = {
            let storage = self.fuzzer.get_struct_storage(self.mutator.spec.checksum);
            self.mutator.dump_graph(&storage)
        };
        let bitmap = self.fuzzer.bitmap_buffer();
        let new_len = dumped_graph.node_len(&self.mutator.spec);
        let mut input = Input::new(
            dumped_graph,
            MutationStrategy::MinimizeSplit,
            vec![],
            Bitmap::new_from_buffer(bitmap),
            exec_res.exitreason,
            std::time::Duration::from_millis(0),
        );
        input.state = InputState::Havoc;

        for (graph_i, storage) in new_bytes.iter().enumerate() {
            if bitmap[storage.index] > 0
                && graphs[graph_i].data.node_len(&self.mutator.spec) > new_len
            {
                input.storage_reasons.push(storage.clone());
                res = true;
            }
        }
        let rc = Rc::new(input);
        for (graph_i, storage) in new_bytes.iter().enumerate() {
            if bitmap[storage.index] > 0
                && graphs[graph_i].data.node_len(&self.mutator.spec) > new_len
            {
                graphs[graph_i] = rc.clone()
            }
        }
        return res;
    }

    pub fn minimize_new_bytes(&mut self, input: &Input) -> HashSet<helpers::HashAsRef<Input>> {
        use std::iter::FromIterator;
        let mut min = input.clone();
        min.id = InputID::invalid();
        min.state = InputState::Havoc;
        let rc = Rc::new(min);
        let new_bytes = rc
            .storage_reasons
            .iter()
            .filter(|r| r.old == 0)
            .cloned()
            .collect::<Vec<_>>();
        let mut graphs = new_bytes.iter().map(|_| rc.clone()).collect::<Vec<_>>();
        drop(rc);

        for i in 0..graphs.len() {
            //println!("Thread {} minimize for {}", self.config.thread_id, i);
            self.minimize_one_byte(i, &new_bytes, &mut graphs);
        }
        return HashSet::from_iter(graphs.into_iter().map(|g| helpers::HashAsRef::new(g)));
    }

    //pub fn minimize_new_bits(&mut self, input: &Input) -> Input {
    //    let num_iters = 1000;
    //    let mut res = input.clone();
    //    let mut remaining = num_iters;
    //    while res.data.node_len(&self.mutator.spec) > 5 && remaining > 0 {
    //        remaining -= 1;
    //        let range = self
    //            .rng
    //            .gen_minimization_block_size(remaining, num_iters, res.data.node_len(&self.mutator.spec));
    //        self.set_input_drop_range(&res.data, range);
    //        let info = self.fuzzer.run_test().unwrap();
    //        let bitmap = self.fuzzer.bitmap_buffer();
    //        if input.storage_reasons.iter().all(|r| r.new >= bitmap[r.index]){
    //            res = self.dump_input(info.exitreason, input.storage_reasons.clone());
    //            println!("minimized input to {}", res.data.node_len(&self.mutator.spec));
    //        }
    //    }
    //    return res;
    //}

    fn minimize_one_byte(
        &mut self,
        i: usize,
        new_bytes: &Vec<StorageReason>,
        graphs: &mut Vec<Rc<Input>>,
    ) {
        let num_iters = 1000;
        let mut remaining = num_iters;
        let mut random_failures = 0;
        while graphs[i].data.node_len(&self.mutator.spec) > 5
            && remaining > 0
            && random_failures < 10
        {
            remaining -= 1;
            //println!(
            //    "Thread {} minimize random {}/1000",
            //    self.config.thread_id, remaining
            //);
            let res = self.perform_run(|mutator, _queue, rng, storage| {
                let range = rng.gen_minimization_block_size(
                    remaining,
                    num_iters,
                    graphs[i].data.node_len(&mutator.spec),
                );
                //println!("drop {:?}", range);
                mutator.drop_range(&graphs[i].data, range, storage, rng);
                MutationStrategy::Minimize
            });

            if let Some((info, _strategy)) = res {
                if self.update_min_graphs(&new_bytes, info, graphs) {
                    //println!("ZOOOOOOOM");
                    random_failures = 0;
                } else {
                    random_failures += 1;
                }
            } else {
                unreachable!();
            }
        }
        //println!(
        //    "after minimized random {}",
        //    graphs[i].data.node_len(&self.mutator.spec)
        //);
        for idx in (0..graphs[i].data.node_len(&self.mutator.spec)).rev() {
            if remaining <= 0 {
                break;
            }
            remaining -= 1;
            //self.set_input_drop_range(&graphs[i].data, idx..idx + 1);
            let res = self.perform_run(|mutator, queue, rng, storage| {
                mutator.drop_range(&graphs[i].data, idx..idx + 1, storage, rng);
                MutationStrategy::Minimize
            });
            if let Some((info, _strategy)) = res {
                self.update_min_graphs(new_bytes, info, graphs);
            }
        }
        //println!(
        //    "after minimized stepwise {}",
        //    graphs[i].data.node_len(&self.mutator.spec)
        //);
    }

    pub fn run(&mut self) {
        use std::io::{self, Write};
        let mut i = 0;
        loop {
            if i == 100 {
                i = 0;
                print!(".");
                io::stdout().flush().unwrap();
            }
            i += 1;
            self.iter();
        }
    }
}
