use crate::data_buff::DataBuff;
use std::ops::Range;

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub enum SizeChangingMutationType {
    DeleteT,
    InsertChunkT,
    InsertFixedT,
    InsertRandomT,
}

impl SizeChangingMutationType {
    pub fn min_size(&self) -> usize {
        use SizeChangingMutationType::*;
        match self {
            DeleteT => return 8,
            InsertChunkT | InsertFixedT | InsertRandomT => return 1,
        }
    }
    pub fn min_available(&self) -> usize {
        use SizeChangingMutationType::*;
        match self {
            DeleteT => return 0,
            InsertChunkT | InsertFixedT | InsertRandomT => return 16,
        }
    }
}

#[derive(Clone, PartialEq, Eq, Hash, Debug)]
pub enum SizeChangingMutation {
    Delete { block: Range<usize> },
    InsertChunk { src: Range<usize>, dst: usize },
    InsertFixed { amount: usize, val: u8, dst: usize },
    InsertRandom { data: Vec<u8>, dst: usize },
}

impl SizeChangingMutation {
    pub fn apply(&self, buff: &mut DataBuff) {
        use SizeChangingMutation::*;
        match self {
            Delete { block } => buff.drop(block),
            InsertChunk { src, dst } => {
                buff.shift_every_after(*dst, src.end - src.start);
                buff.copy_within(&src, *dst);
            }
            InsertFixed { dst, amount, val } => {
                buff.shift_every_after(*dst, *amount);
                buff.memset(&(*dst..dst + amount), *val);
            }
            InsertRandom { data, dst } => {
                buff.shift_every_after(*dst, data.len());
                buff.copy_from(data, *dst);
            }
        }
    }
}
