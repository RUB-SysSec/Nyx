use crate::random::distributions::Distributions;

#[derive(Debug, PartialEq, Deserialize, Serialize, Clone)]
#[serde(tag = "type")]
pub enum IntGenerator{
    Options{opts: Vec<u64>},
    Flags{opts: Vec<u64>},
    Limits{range:(u64, u64), align: u64},
}

impl IntGenerator{
    pub fn generate(&self, dist: &Distributions) -> (u64,bool){
        use IntGenerator::*;
        match self {
            Options{opts} => (opts[dist.gen_range(0,opts.len())],false),
            Flags{opts} => (opts[dist.gen_range(0,opts.len())],false),
            Limits{range,align} => {
                let mut val = dist.gen_range(range.0,range.1);
                val = val-(val%align);
                if val < range.0 {val+=align}
                if val > range.1 {val-=align}
                (val,false)
            },
        }
    }
}