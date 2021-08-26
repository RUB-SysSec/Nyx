use std::sync::Arc;
use std::sync::RwLock;
use std::time::Duration;

use crate::structured_fuzzer::custom_dict::CustomDict;
use crate::bitmap::{Bitmap, StorageReason};
use crate::fuzz_runner::ExitReason;
use crate::structured_fuzzer::graph_mutator::graph_storage::VecGraph;
use crate::structured_fuzzer::mutator::MutationStrategy;

#[derive(Clone, Copy, Debug, Eq, PartialEq, Hash)]
pub struct InputID(usize);

impl InputID {
    pub fn new(a: usize) -> Self {
        Self(a)
    }
    pub fn invalid() -> Self {
        Self(std::usize::MAX)
    }
    pub fn as_usize(&self) -> usize {
        self.0
    }
}

#[derive(Clone)]
pub enum InputState {
    Minimize,
    Havoc,
}

#[derive(Clone)]
pub struct Input {
    pub id: InputID,
    pub data: Arc<VecGraph>,
    pub bitmap: Bitmap,
    pub exit_reason: ExitReason,
    pub time: Duration,
    pub storage_reasons: Vec<StorageReason>,
    pub found_by: MutationStrategy,
    pub state: InputState,
    pub custom_dict: CustomDict,
}

impl Input {
    pub fn new(
        data: VecGraph,
        found_by: MutationStrategy,
        storage_reasons: Vec<StorageReason>,
        bitmap: Bitmap,
        exit_reason: ExitReason,
        time: Duration,
    ) -> Self {
        return Self {
            id: InputID::invalid(),
            data: Arc::new(data),
            bitmap,
            storage_reasons,
            exit_reason,
            time,
            state: InputState::Minimize,
            found_by,
            custom_dict: CustomDict::new(),
        };
    }
}
