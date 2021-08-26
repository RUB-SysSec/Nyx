use rand::SeedableRng;
use rand_core::{impls, Error, RngCore};

pub struct RomuPrng {
    xstate: u64,
    ystate: u64,
}

impl RomuPrng {
    pub fn new(xstate: u64, ystate: u64) -> Self {
        return Self { xstate, ystate };
    }
}

impl SeedableRng for RomuPrng {
    type Seed = [u8; 16];

    fn from_seed(seed: [u8; 16]) -> RomuPrng {
        if seed == [0; 16] {
            return RomuPrng::new(0x0DDB1A5E5BAD5EEDu64, 0x519fb20ce6a199bbu64);
        }
        let x = u64::from_le_bytes([
            seed[0], seed[1], seed[2], seed[3], seed[4], seed[5], seed[6], seed[7],
        ]);
        let y = u64::from_le_bytes([
            seed[8], seed[9], seed[10], seed[11], seed[12], seed[13], seed[14], seed[15],
        ]);
        return RomuPrng::new(x, y);
    }
}

impl RngCore for RomuPrng {
    fn next_u32(&mut self) -> u32 {
        self.next_u64() as u32
    }

    fn next_u64(&mut self) -> u64 {
        let xp = self.xstate;
        self.xstate = 15241094284759029579u64.wrapping_mul(self.ystate);
        self.ystate = self.ystate.wrapping_sub(xp);
        self.ystate = self.ystate.rotate_left(27);
        return xp;
    }

    fn fill_bytes(&mut self, dest: &mut [u8]) {
        impls::fill_bytes_via_next(self, dest)
    }

    fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), Error> {
        Ok(self.fill_bytes(dest))
    }
}
