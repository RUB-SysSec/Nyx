use std::convert::TryInto;
use std::ops::Range;

use crate::random::distributions::Distributions;

pub struct DataBuff<'a> {
    data: &'a mut [u8],
    used: usize,
}

impl<'a> DataBuff<'a> {
    pub fn new(data: &'a mut [u8], used: usize) -> DataBuff<'a> {
        return DataBuff { data, used };
    }

    pub fn memset(&mut self, range: &Range<usize>, val: u8) {
        for i in range.clone() {
            self.data[i] = val;
        }
    }

    pub fn set_to_random(&mut self, len: usize, prng: &Distributions) {
        self.clear();
        self.used = len;
        for i in 0..len {
            self.data[i] = prng.gen();
        }
    }

    pub fn set_to_slice(&mut self, data: &[u8]) {
        self.clear();
        self.used = data.len();
        self.data[..data.len()].copy_from_slice(data);
    }

    pub fn expand_to(&mut self, size: usize) {
        assert!(size <= self.data.len());
        if self.used < size {
            self.memset(&(self.used..size), 0);
            self.used = size;
        }
    }

    pub fn copy_within(&mut self, src: &Range<usize>, dst: usize) {
        self.expand_to(dst + src.len());
        self.data.copy_within(src.clone(), dst);
    }

    pub fn copy_from(&mut self, data: &[u8], dst: usize) {
        self.expand_to(dst + data.len());
        self.data[dst..dst + data.len()].copy_from_slice(data)
    }

    /// copies all bytes after offset by amount bytes to the right.
    /// The space between offset and offset+amount is unmodified (or initialized with zeros if previously unused)
    /// Example: ABC|DEFGH amount: 3 => ABCDEFDEFGH
    pub fn shift_every_after(&mut self, offset: usize, amount: usize) {
        self.copy_within(&(offset..self.len()), offset + amount);
    }

    pub fn append(&mut self, data: &[u8]) {
        self.data[self.used..self.used + data.len()].copy_from_slice(data);
    }

    /// removes the bytes at the given range.
    /// Example: ABC[DEFG]HIJ => ABCHIJ
    pub fn drop(&mut self, src: &Range<usize>) {
        let rest = self.used - src.end;
        self.copy_within(&(src.end..self.used), src.start);
        self.shrink(src.start + rest);
    }

    pub fn shrink(&mut self, new_size: usize) {
        assert!(new_size <= self.used);
        self.used = new_size;
    }

    pub fn clear(&mut self) {
        self.shrink(0);
    }

    /// returns the number of bytes that are currently in use
    pub fn len(&self) -> usize {
        return self.used;
    }

    /// returns the number of bytes in the backend data store
    pub fn capacity(&self) -> usize {
        return self.data.len();
    }

    // returns the number of bytes currently not in use
    pub fn available(&self) -> usize {
        return self.capacity() - self.len();
    }

    pub fn as_slice(&self) -> &[u8] {
        return &self.data[..self.used];
    }

    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        return &mut self.data[..self.used];
    }

    pub fn write_u8(&mut self, offset: usize, val: u8) {
        assert!(offset < self.used);
        self.data[offset] = val;
    }
    pub fn read_u8(&self, offset: usize) -> u8 {
        assert!(offset < self.used);
        return self.data[offset];
    }

    pub fn read_u16(&self, offset: usize, flip_endianess: bool) -> u16 {
        assert!(offset + 2 <= self.used);
        let v: [u8; 2] = self.data[offset..offset + 2].try_into().unwrap();
        let res = u16::from_le_bytes(v);
        return if flip_endianess {
            res.swap_bytes()
        } else {
            res
        };
    }

    pub fn write_u16(&mut self, offset: usize, flip_endianess: bool, val: u16) {
        assert!(offset + 2 <= self.used);
        let res = if flip_endianess {
            val.swap_bytes()
        } else {
            val
        };
        self.data[offset..offset + 2].copy_from_slice(&res.to_le_bytes());
    }

    pub fn read_u32(&self, offset: usize, flip_endianess: bool) -> u32 {
        assert!(offset + 4 <= self.used);
        let v: [u8; 4] = self.data[offset..offset + 4].try_into().unwrap();
        let res = u32::from_le_bytes(v);
        return if flip_endianess {
            res.swap_bytes()
        } else {
            res
        };
    }

    pub fn write_u32(&mut self, offset: usize, flip_endianess: bool, val: u32) {
        assert!(offset + 4 <= self.used);
        let res = if flip_endianess {
            val.swap_bytes()
        } else {
            val
        };
        self.data[offset..offset + 4].copy_from_slice(&res.to_le_bytes());
    }

    pub fn read_u64(&self, offset: usize, flip_endianess: bool) -> u64 {
        assert!(offset + 8 <= self.used);
        let v: [u8; 8] = self.data[offset..offset + 8].try_into().unwrap();
        let res = u64::from_le_bytes(v);
        return if flip_endianess {
            res.swap_bytes()
        } else {
            res
        };
    }

    pub fn write_u64(&mut self, offset: usize, flip_endianess: bool, val: u64) {
        assert!(offset + 8 <= self.used);
        let res = if flip_endianess {
            val.swap_bytes()
        } else {
            val
        };
        self.data[offset..offset + 8].copy_from_slice(&res.to_le_bytes());
    }
}
