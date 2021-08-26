use std::i64;
use std::ops::Range;
use std::rc::Rc;

use crate::data_buff::DataBuff;
use crate::custom_dict::CustomDict;
use crate::primitive_mutator::inplace_mutation::{InplaceMutation, InplaceMutationType};
use crate::primitive_mutator::size_changing_mutation::{
    SizeChangingMutation, SizeChangingMutationType,
};
use crate::random::distributions::Distributions;

const INTERESTING_U8: [u8; 9] = [(-128i8) as u8, (-1i8) as u8, 0, 1, 16, 32, 64, 100, 127];
const INTERESTING_U16: [u16; 19] = [
    (-128i16) as u16,
    (-1i16) as u16,
    0,
    1,
    16,
    32,
    64,
    100,
    127, //u8
    (-32768i16) as u16,
    (-129i16) as u16,
    128,
    255,
    256,
    512,
    1000,
    1024,
    4096,
    32767,
];
const INTERESTING_U32: [u32; 27] = [
    (-128i32) as u32,
    (-1i32) as u32,
    0,
    1,
    16,
    32,
    64,
    100,
    127, //u8
    (-32768i32) as u32,
    (-129i32) as u32,
    128,
    255,
    256,
    512,
    1000,
    1024,
    4096,
    32767, //u16
    (-2147483648i32) as u32,
    (-100663046i32) as u32,
    (-32769i32) as u32,
    32768,
    65535,
    65536,
    100663045,
    2147483647,
];
const INTERESTING_U64: [u64; 30] = [
    (-128i64) as u64,
    (-1i64) as u64,
    0,
    1,
    16,
    32,
    64,
    100,
    127, //u8
    (-32768i32) as u64,
    (-129i64) as u64,
    128,
    255,
    256,
    512,
    1000,
    1024,
    4096,
    32767, //u16
    (-2147483648i64) as u64,
    (-100663046i64) as u64,
    (-32769i64) as u64,
    32768,
    65535,
    65536,
    100663045,
    2147483647, //u32
    i64::MIN as u64,
    0x7fffffffffffffff,
    0x8080808080808080,
];

#[derive(Clone, Eq, Hash, PartialEq, Debug)]
pub enum Mutation {
    Inplace(InplaceMutation),
    SizeChanging(SizeChangingMutation),
}

impl Mutation {
    pub fn apply(&self, buff: &mut DataBuff) {
        match self {
            Mutation::Inplace(x) => x.apply(buff),
            Mutation::SizeChanging(x) => x.apply(buff),
        }
    }
}

pub struct PrimitiveMutator {}

impl PrimitiveMutator {
    pub fn new() -> Self {
        return Self { };
    }

    pub fn mutate(&self, buff: &mut DataBuff, dict: Option<&CustomDict>, dist: &Distributions) {
        if buff.len() == 0 {
            return;
        }
        if let Some(dict) = dict{
            if  dist.should_mutate_dict() {
                let continue_mutation = dict.mutate(buff, dist);
                if ! continue_mutation {return}
            }
        }
        //TODO add size changing mutations if buff.available != 0
        let mutation = self.gen_inplace_mutation(buff,dist);
        mutation.apply(buff);
    }

    fn gen_inplace_mutation_type(&self, buff: &DataBuff,dist: &Distributions) -> InplaceMutationType {
        assert!(buff.len() > 0);
        for _ in 1..5 {
            let t = *dist.gen_inplace_mutation_type();
            if t.min_size() <= buff.len() {
                return t;
            }
        }
        return InplaceMutationType::FlipBitT;
    }

    pub fn gen_inplace_mutation(&self, buff: &DataBuff,dist: &Distributions) -> InplaceMutation {
        use InplaceMutation::*;
        use InplaceMutationType::*;
        let m_type = self.gen_inplace_mutation_type(buff,dist);
        match m_type {
            FlipBitT => FlipBit {
                offset: self.gen_offset(1, buff, dist),
                bit: dist.gen_range(0, 8),
            },
            AddU8T => AddU8 {
                offset: self.gen_offset(1, buff, dist),
                val: self.gen_arith_val(dist) as u8,
            },
            AddU16T => AddU16 {
                offset: self.gen_offset(2, buff, dist),
                val: self.gen_arith_val(dist) as u16,
                flip_endian: dist.gen_endianess(),
            },
            AddU32T => AddU32 {
                offset: self.gen_offset(4, buff, dist),
                val: self.gen_arith_val(dist) as u32,
                flip_endian: dist.gen_endianess(),
            },
            AddU64T => AddU64 {
                offset: self.gen_offset(8, buff, dist),
                val: self.gen_arith_val(dist) as u64,
                flip_endian: dist.gen_endianess(),
            },
            InterestingU8T => InterestingU8 {
                offset: self.gen_offset(1, buff, dist),
                val: self.gen_pick(&INTERESTING_U8, dist),
            },
            InterestingU16T => InterestingU16 {
                offset: self.gen_offset(2, buff, dist),
                val: self.gen_pick(&INTERESTING_U16, dist),
                flip_endian: dist.gen_endianess(),
            },
            InterestingU32T => InterestingU32 {
                offset: self.gen_offset(4, buff, dist),
                val: self.gen_pick(&INTERESTING_U32, dist),
                flip_endian: dist.gen_endianess(),
            },
            InterestingU64T => InterestingU64 {
                offset: self.gen_offset(8, buff, dist),
                val: self.gen_pick(&INTERESTING_U64, dist),
                flip_endian: dist.gen_endianess(),
            },
            OverwriteRandomByteT => {
                let offset = self.gen_offset(1, buff, dist);
                OverwriteRandomByte {
                    offset,
                    val: dist.gen_range(1, 0xff) ^ buff.read_u8(offset),
                }
            }
            OverwriteChunkT => {
                let src = self.gen_block_range(buff, dist);
                let len = src.end - src.start;
                let mut dst = self.gen_offset(len, buff, dist);
                while src.start == dst && len != buff.len() {
                    dst = self.gen_offset(len, buff, dist);
                }
                OverwriteChunk { src, dst }
            }
            OverwriteRandomT => {
                let dst = self.gen_block_range(buff, dist);
                let data = dst.clone().map(|_| dist.gen()).collect();
                OverwriteRandom {
                    data,
                    dst: dst.start,
                }
            }
            OverwriteFixedT => OverwriteFixed {
                block: self.gen_block_range(buff, dist),
                val: dist.gen(),
            },
        }
    }

    fn gen_size_changing_mutation_type(&self, buff: &DataBuff,dist: &Distributions) -> SizeChangingMutationType {
        assert!(buff.capacity() > 16);
        assert!(buff.len() > 0);
        for _ in 1..5 {
            let t = *dist.gen_size_changing_mutation_type();
            if t.min_size() <= buff.len() && t.min_available() < buff.available() {
                return t;
            }
        }
        if buff.available() > SizeChangingMutationType::InsertRandomT.min_available() {
            return SizeChangingMutationType::InsertRandomT;
        }
        return SizeChangingMutationType::DeleteT;
    }

    pub fn gen_size_changing_mutation(&self, buff: &DataBuff,dist: &Distributions) -> SizeChangingMutation {
        use SizeChangingMutation::*;
        use SizeChangingMutationType::*;
        let m_type = self.gen_size_changing_mutation_type(buff, dist);
        match m_type {
            DeleteT => Delete {
                block: self.gen_block_range(buff, dist),
            },
            InsertChunkT => InsertChunk {
                src: self.gen_insert_block_range(buff, dist),
                dst: self.gen_offset(1, buff, dist),
            },
            InsertFixedT => InsertFixed {
                dst: self.gen_offset(1, buff, dist),
                amount: dist.gen_range(1, buff.capacity() - buff.len()),
                val: dist.gen(),
            },
            InsertRandomT => {
                let max = dist.gen_range(1, buff.available());
                InsertRandom {
                    data: (0..max).map(|_| dist.gen()).collect(),
                    dst: self.gen_offset(1, buff, dist),
                }
            }
        }
    }

    pub fn gen_mutation(&self, buff: &DataBuff,dist: &Distributions) -> Mutation {
        if dist.gen() {
            return Mutation::Inplace(self.gen_inplace_mutation(buff, dist));
        }
        return Mutation::SizeChanging(self.gen_size_changing_mutation(buff, dist));
    }

    fn gen_offset(&self, size: usize, buff: &DataBuff,dist: &Distributions) -> usize {
        assert!(buff.len() >= size);
        dist.gen_range(0, buff.len() - size + 1)
    }

    fn gen_arith_val(&self,dist: &Distributions) -> i8 {
        if dist.gen() {
            dist.gen_range(1, 35)
        } else {
            dist.gen_range(-35, -1)
        }
    }

    fn gen_pick<T: Copy>(&self, data: &[T],dist: &Distributions) -> T {
        data[dist.gen_range(0, data.len())]
    }
    fn gen_insert_block_range(&self, buff: &DataBuff,dist: &Distributions) -> Range<usize> {
        return self.gen_block_max(0..buff.len(), buff.capacity() - buff.len(), dist);
    }
    fn gen_block_range(&self, buff: &DataBuff,dist: &Distributions) -> Range<usize> {
        return self.gen_block_max(0..buff.len(), buff.len(), dist);
    }
    fn gen_block_max(&self, range: Range<usize>, max_len: usize,dist: &Distributions) -> Range<usize> {
        if range.start == range.end {
            return range;
        }
        let (mut min, mut max) = *dist.gen_block_size();
        if max > max_len {
            max = max_len;
        }
        let len = range.end - range.start;
        if max > len {
            max = len;
        }
        if min >= max {
            min = 1;
        }
        let size = dist.gen_range(min, max + 1);
        let start = dist.gen_range(0, range.end - size + 1);
        return start..start + size;
    }

    pub fn gen_num_array_elems(
        &self,
        elem_size: usize,
        min_elems: usize,
        max_elems: usize,
        max_data: usize,
        dist: &Distributions
    ) -> usize {
        if max_data / elem_size > min_elems {
            return dist
                .gen_range(min_elems, max_elems.min(max_data / elem_size));
        }
        return max_data / elem_size;
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_inplace() {
        let dist = crate::random::distributions::Distributions::new();
        let mutator = PrimitiveMutator::new();
        let mut data = vec![0u8; 1024];
        let mut buff = DataBuff::new(&mut data, 0);

        for _ in 0..1000 {
            let len = dist.gen_range(1, 1024);
            buff.set_to_random(len, &dist);
            let mutation = mutator.gen_inplace_mutation(&buff,&dist);

            mutation.apply(&mut buff);
            assert_eq!(buff.len(), len);
        }
    }

    #[test]
    fn test_sized() {
        let dist = crate::random::distributions::Distributions::new();
        let mutator = PrimitiveMutator::new();
        let mut data = vec![0u8; 1024];
        let mut buff = DataBuff::new(&mut data, 0);

        for _ in 0..1000 {
            let len = dist.gen_range(1, 1024);
            buff.set_to_random(len, &dist);
            let mutation = mutator.gen_size_changing_mutation(&buff, &dist);
            println!("{:?} on buff of length {}", mutation, buff.len());
            mutation.apply(&mut buff);
        }
    }

    use std::collections::HashMap;
    #[test]
    fn test_uniqueness() {
        let dist = crate::random::distributions::Distributions::new();
        let mutator = PrimitiveMutator::new();
        let mut data = vec![0u8; 1024];
        let mut buff = DataBuff::new(&mut data, 0);
        let mut storage = HashMap::<Vec<u8>, usize>::new();
        let mut infos = HashMap::<Vec<u8>, HashMap<Mutation, usize>>::new();
        let base = (0..256)
            .map(|_| dist.gen::<u8>())
            .collect::<Vec<_>>();
        let iters = 10000;
        for _ in 0..iters {
            buff.set_to_slice(&base);
            let mutation = mutator.gen_mutation(&buff, &dist);
            mutation.apply(&mut buff);
            let inf = infos
                .entry(buff.as_slice().to_vec())
                .or_insert_with(|| HashMap::new());
            *(inf.entry(mutation).or_insert(0)) += 1;
            *storage.entry(buff.as_slice().to_vec()).or_insert(0) += 1;
        }
        println!("{} of {} iters produced uniq results", storage.len(), iters);
        let mut generated = storage.keys().collect::<Vec<_>>();
        generated.sort_unstable_by_key(|x| std::usize::MAX - storage.get(*x).unwrap());
        for dat in generated.iter().take(20) {
            println!(
                "data was hit {:?} times, produced by {:?}",
                storage.get(*dat),
                infos.get(*dat)
            );
        }
        assert!(storage.len() >= iters - (iters / 5)); //max 5% of the mutation should be duplicates
    }
}
