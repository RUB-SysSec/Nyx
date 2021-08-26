use crate::bitmap::{BitmapHandler, StorageReason};
use crate::config::FuzzerConfig;
use crate::fuzz_runner::ExitReason;
use crate::structured_fuzzer::custom_dict::CustomDict;
use crate::input::{Input, InputID, InputState};
use crate::structured_fuzzer::graph_mutator::graph_storage::{GraphStorage, VecGraph};
use crate::structured_fuzzer::mutator::InputQueue;
use crate::structured_fuzzer::random::distributions::Distributions;
use crate::structured_fuzzer::GraphSpec;

use std::collections::HashMap;

use std::sync::Arc;
use std::sync::RwLock;

#[derive(Serialize)]
pub struct QueueStats {
    num_inputs: usize,
    favqueue: Vec<usize>,
}

pub struct QueueData {
    bitmap_index_to_min_example: HashMap<usize, InputID>,
    bitmap_index_to_max_count: HashMap<usize, InputID>,
    favqueue: Vec<InputID>,
    inputs: Vec<Arc<RwLock<Input>>>,
    bitmap_bits: Vec<usize>,
    bitmaps: BitmapHandler,
    next_input_id: usize,
}

#[derive(Clone)]
pub struct Queue {
    workdir: String,
    start_time: std::time::Instant,
    data: Arc<RwLock<QueueData>>,
}

impl<'a> InputQueue for Queue {
    fn sample_for_splicing(&self, dist: &Distributions) -> Arc<VecGraph> {
        let data_lock = self.data.read().unwrap();
        let inputs = &data_lock.inputs;
        let i = dist.gen_range(0, inputs.len());
        let inp = inputs[i].read().unwrap().data.clone();
        return inp;
    }
}

impl Queue {
    pub fn new(config: &FuzzerConfig) -> Self {
        return Self {
            workdir: config.workdir_path.clone(),
            start_time: std::time::Instant::now(),
            data: Arc::new(RwLock::new(QueueData {
                bitmap_index_to_min_example: HashMap::new(),
                bitmap_index_to_max_count: HashMap::new(),
                inputs: vec![],
                favqueue: vec![],
                bitmap_bits: vec![],
                bitmaps: BitmapHandler::new(config.bitmap_size),
                next_input_id: 0,
            })),
        };
    }

    pub fn write_stats(&self) {
        use std::fs::File;
        use std::fs::OpenOptions;
        use std::io::prelude::*;
        let dat = self.data.read().unwrap();
        let ser = QueueStats {
            num_inputs: dat.inputs.len(),
            favqueue: dat
                .favqueue
                .iter()
                .map(|id| id.as_usize())
                .collect::<Vec<_>>(),
        };
        let mut file = File::create(format!("{}/queue_stats.msgp", &self.workdir)).unwrap();
        rmp_serde::encode::write_named(&mut file, &ser).unwrap();

        let mut file = OpenOptions::new()
            .append(true)
            .create(true)
            .open(format!("{}/bitmap_stats.txt", &self.workdir))
            .unwrap();
        file.write_fmt(format_args!(
            "{},{}\n",
            (std::time::Instant::now() - self.start_time).as_secs_f32(),
            dat.bitmaps.normal_bitmap().bits().iter().filter(|b| **b > 0).count()
        ))
        .unwrap();
    }

    pub fn len(&self) -> usize {
        return self.data.read().unwrap().inputs.len();
    }

    pub fn should_minimize(&self, inp: &Input) -> bool {
        match inp.exit_reason {
            ExitReason::Normal(_) => return inp.storage_reasons.iter().any(|rea| rea.old == 0),
            _ => return false,
        }
    }

    pub fn check_new_bytes(
        &mut self,
        run_bitmap: &[u8],
        etype: &ExitReason,
    ) -> Option<Vec<StorageReason>> {
        self.data
            .write()
            .unwrap()
            .bitmaps
            .check_new_bytes(run_bitmap, etype)
    }

    pub fn set_state(&mut self, inp: &Input, state: InputState) {
        let mut data = self.data.write().unwrap();
        assert!(inp.id.as_usize() < data.inputs.len());
        data.inputs[inp.id.as_usize()].write().unwrap().state = state;
    }

    pub fn set_custom_dict(&mut self, inp: &Input, custom_dict: CustomDict){
        let mut data = self.data.write().unwrap();
        assert!(inp.id.as_usize() < data.inputs.len());
        data.inputs[inp.id.as_usize()].write().unwrap().custom_dict = custom_dict;
    }

    pub fn add(&mut self, mut input: Input, spec: &GraphSpec) -> Option<InputID> {
        assert_eq!(input.id, InputID::invalid());
        if input.data.node_len(spec) == 0 {
            return None;
        }
        let id;
        match input.exit_reason {
                ExitReason::Normal(_) | ExitReason::Crash(_) => {
                let has_new_bytes = input.storage_reasons.iter().any(|r| r.old == 0);
                let should_update_favs;
                {
                    let mut data = self.data.write().unwrap();
                    should_update_favs = has_new_bytes;
                    id = InputID::new(data.inputs.len());
                    input.id = id;
                    let new_len = input.data.node_len(&spec);
                    let input = Arc::new(RwLock::new(input));
                    data.inputs.push(input.clone());
                    for reason in input.read().unwrap().storage_reasons.iter() {
                        if !data.bitmap_index_to_min_example.contains_key(&reason.index) {
                            data.bitmap_bits.push(reason.index);
                            data.bitmap_index_to_min_example.insert(reason.index, id);
                        }
                        let old_entry = data
                            .bitmap_index_to_min_example
                            .get_mut(&reason.index)
                            .unwrap()
                            .as_usize();
                        if data.inputs[old_entry].read().unwrap().data.node_len(&spec) > new_len {
                            data.bitmap_index_to_min_example.insert(reason.index, id);
                        }
                    }
                }
                //if should_update_favs {
                    self.calc_fav_bits();
                //}
                self.write_stats();
            }
            /*
            ExitReason::Crash(_) => return None, //println!("NEW crash found!"),
            */
            _ => {
                println!("ignoring input {:?}", input.exit_reason);
                return None;
            }
        }
        return Some(id);
    }

    pub fn calc_fav_bits(&mut self) {
        let mut favids = vec![];
        {
            let mut data = self.data.read().unwrap();
            let mut bits = vec![0u8; data.bitmaps.size()];
            for input in data.inputs.iter().rev() {
                let inp = input.read().unwrap();
                if inp.storage_reasons.iter().any(|s| s.old == 0) {
                    //found new bytes
                    if inp
                        .bitmap
                        .bits()
                        .iter()
                        .enumerate()
                        .any(|(i, v)| bits[i] == 0 && *v > 0)
                    {
                        for (i, v) in inp.bitmap.bits().iter().enumerate() {
                            if *v != 0 {
                                bits[i] = 1;
                            }
                        }
                        favids.push(inp.id);
                    }
                }
            }
        }
        {
            let mut data = self.data.write().unwrap();
            println!(
                "calc favbits ({}) out of {}",
                favids.len(),
                data.inputs.len()
            );
            data.favqueue = favids;
        }
    }

    pub fn schedule(&self, rng: &Distributions) -> Arc<RwLock<Input>> {
        let data = self.data.read().unwrap();
        if data.favqueue.len() > 0 && rng.gen::<u32>() % 10 <= 5 {
            let id = &data.favqueue[rng.gen_range(0, data.favqueue.len())];
            return data.inputs[id.as_usize()].clone();
        }
        if rng.gen::<u32>() % 8 <= 5 && data.bitmap_bits.len() != 0 {
            let bit = &data.bitmap_bits[rng.gen_range(0, data.bitmap_bits.len())];
            return data.inputs[data
                .bitmap_index_to_min_example
                .get(bit)
                .unwrap()
                .as_usize()]
            .clone();
        } else {
            return data.inputs[rng.gen_range(0, data.inputs.len())].clone();
        }
    }

    pub fn next_id(&mut self) -> usize {
        let mut data = self.data.write().unwrap();
        data.next_input_id += 1;
        return data.next_input_id;
    }

    pub fn get_input(&self, id: InputID) -> Arc<RwLock<Input>> {
        let data = self.data.read().unwrap();
        return data.inputs[id.as_usize()].clone();
    }
}
