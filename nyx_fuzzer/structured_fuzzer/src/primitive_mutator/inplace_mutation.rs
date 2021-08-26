use crate::data_buff::DataBuff;
use std::ops::Range;

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub enum InplaceMutationType {
    FlipBitT,
    AddU8T,
    AddU16T,
    AddU32T,
    AddU64T,
    InterestingU8T,
    InterestingU16T,
    InterestingU32T,
    InterestingU64T,
    OverwriteRandomByteT,
    OverwriteChunkT,
    OverwriteRandomT,
    OverwriteFixedT,
}

impl InplaceMutationType {
    pub fn min_size(&self) -> usize {
        use InplaceMutationType::*;
        match self {
            AddU8T | InterestingU8T | FlipBitT | OverwriteRandomByteT | OverwriteRandomT
            | OverwriteFixedT => return 1,
            AddU16T | InterestingU16T | OverwriteChunkT => return 2,
            AddU32T | InterestingU32T => return 4,
            AddU64T | InterestingU64T => return 8,
        }
    }
}

#[derive(Clone, PartialEq, Eq, Hash, Debug)]
pub enum InplaceMutation {
    FlipBit {
        offset: usize,
        bit: usize,
    },
    AddU8 {
        offset: usize,
        val: u8,
    },
    AddU16 {
        offset: usize,
        val: u16,
        flip_endian: bool,
    },
    AddU32 {
        offset: usize,
        val: u32,
        flip_endian: bool,
    },
    AddU64 {
        offset: usize,
        val: u64,
        flip_endian: bool,
    },
    InterestingU8 {
        offset: usize,
        val: u8,
    },
    InterestingU16 {
        offset: usize,
        val: u16,
        flip_endian: bool,
    },
    InterestingU32 {
        offset: usize,
        val: u32,
        flip_endian: bool,
    },
    InterestingU64 {
        offset: usize,
        val: u64,
        flip_endian: bool,
    },
    OverwriteRandomByte {
        offset: usize,
        val: u8,
    },
    OverwriteChunk {
        src: Range<usize>,
        dst: usize,
    },
    OverwriteRandom {
        data: Vec<u8>,
        dst: usize,
    },
    OverwriteFixed {
        block: Range<usize>,
        val: u8,
    },
}

impl InplaceMutation {
    pub fn apply(&self, buff: &mut DataBuff) {
        use InplaceMutation::*;
        match self {
            FlipBit { offset, bit } => {
                let new = buff.read_u8(*offset) ^ (1u8 << bit);
                buff.write_u8(*offset, new)
            }
            AddU8 { offset, val } => {
                let new = buff.read_u8(*offset).wrapping_add(*val);
                buff.write_u8(*offset, new)
            }
            AddU16 {
                offset,
                val,
                flip_endian,
            } => {
                let new = buff.read_u16(*offset, *flip_endian).wrapping_add(*val);
                buff.write_u16(*offset, *flip_endian, new)
            }
            AddU32 {
                offset,
                val,
                flip_endian,
            } => {
                let new = buff.read_u32(*offset, *flip_endian).wrapping_add(*val);
                buff.write_u32(*offset, *flip_endian, new)
            }
            AddU64 {
                offset,
                val,
                flip_endian,
            } => {
                let new = buff.read_u64(*offset, *flip_endian).wrapping_add(*val);
                buff.write_u64(*offset, *flip_endian, new)
            }
            InterestingU8 { offset, val } => buff.write_u8(*offset, *val),
            InterestingU16 {
                offset,
                val,
                flip_endian,
            } => buff.write_u16(*offset, *flip_endian, *val),
            InterestingU32 {
                offset,
                val,
                flip_endian,
            } => buff.write_u32(*offset, *flip_endian, *val),
            InterestingU64 {
                offset,
                val,
                flip_endian,
            } => buff.write_u64(*offset, *flip_endian, *val),
            OverwriteRandomByte { offset, val } => buff.write_u8(*offset, *val),
            OverwriteRandom { data, dst } => buff.copy_from(&data, *dst),
            OverwriteFixed { block, val } => buff.memset(block, *val),
            OverwriteChunk { src, dst } => {
                buff.copy_within(src, *dst);
            }
        }
    }
}
