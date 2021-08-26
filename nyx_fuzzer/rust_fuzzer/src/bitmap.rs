use crate::fuzz_runner::ExitReason;

#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub struct StorageReason {
    pub index: usize,
    pub old: u8,
    pub new: u8,
}

pub struct BitmapHandler {
    normal: Bitmap,
    crash: Bitmap,
    timeout: Bitmap,
    invalid_write_to_payload: Bitmap,
    size: usize,
}

impl BitmapHandler {
    pub fn new(size: usize) -> Self {
        return Self {
            normal: Bitmap::new(size),
            crash: Bitmap::new(size),
            timeout: Bitmap::new(size),
            invalid_write_to_payload: Bitmap::new(size),
            size,
        };
    }

    pub fn check_new_bytes(
        &mut self,
        run_bitmap: &[u8],
        etype: &ExitReason,
    ) -> Option<Vec<StorageReason>> {
        match etype {
            ExitReason::Normal(_) => return self.normal.check_new_bytes(run_bitmap),
            ExitReason::Crash(_) => return self.crash.check_new_bytes(run_bitmap),
            ExitReason::Timeout => return self.timeout.check_new_bytes(run_bitmap),
            ExitReason::InvalidWriteToPayload(_) => {
                return self.invalid_write_to_payload.check_new_bytes(run_bitmap)
            }
            _ => return None,
        }
    }

    pub fn size(&self) -> usize {
        self.size
    }
    
    pub fn normal_bitmap(&self) -> &Bitmap{
        return &self.normal
    }
}

#[derive(Clone)]
pub struct Bitmap {
    bits: Vec<u8>,
}

impl Bitmap {
    pub fn new(size: usize) -> Self {
        return Self {
            bits: vec![0; size],
        };
    }

    pub fn new_from_buffer(buff: &[u8]) -> Self {
        return Self {
            bits: buff.to_vec(),
        };
    }

    pub fn check_new_bytes(&mut self, run_bitmap: &[u8]) -> Option<Vec<StorageReason>> {
        assert_eq!(self.bits.len(), run_bitmap.len());
        let mut res = None;
        for (i, (old, new)) in self.bits.iter_mut().zip(run_bitmap.iter()).enumerate() {
            if *new > *old {
                if res.is_none() {
                    res = Some(vec![]);
                }
                res.as_mut().unwrap().push(StorageReason {
                    index: i,
                    old: *old,
                    new: *new,
                });
                *old = *new;
            }
        }
        return res;
    }

    pub fn bits(&self) -> &[u8] {
        return &self.bits;
    }
}
