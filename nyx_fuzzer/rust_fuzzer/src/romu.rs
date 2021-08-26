pub struct RomuPrng {
    xstate: u64,
    ystate: u64,
}

impl RomuPrng {
    pub fn new(xstate: u64, ystate: u64) -> Self {
        return Self { xstate, ystate };
    }

    pub fn range(&mut self, min: usize, max: usize) -> usize {
        return ((self.next_u64() as usize) % (max - min)) + min;
    }

    pub fn new_from_u64(seed: u64) -> Self {
        return Self::new(seed, seed ^ 0xec77152282650854);
    }

    pub fn next_u32(&mut self) -> u32 {
        self.next_u64() as u32
    }

    pub fn next_u64(&mut self) -> u64 {
        let xp = self.xstate;
        self.xstate = 15241094284759029579u64.wrapping_mul(self.ystate);
        self.ystate = self.ystate.wrapping_sub(xp);
        self.ystate = self.ystate.rotate_left(27);
        return xp;
    }
}
