use std::collections::HashMap;

use crate::data_buff::DataBuff;
use crate::random::distributions::Distributions;

#[derive(Clone)]
pub enum DictEntry{
    Replace(Vec<u8>, Vec<u8>)
}

#[derive(Clone)]
pub struct CustomDict{
    groups: Vec<Vec<DictEntry>>,
    lhs_to_groups: HashMap<Vec<u8>,Vec<Vec<u8>>>,
}

impl CustomDict{
    pub fn new() -> Self{
        return Self{groups: vec!(), lhs_to_groups: HashMap::new()}
    }

    pub fn new_from_groups(groups: Vec<Vec<DictEntry>>) -> Self{
        let mut lhs_to_groups = HashMap::new();
        for group in groups.iter() {
            for entry in group.iter() {
                match entry {
                    DictEntry::Replace(lhs, rhs) => {
                        let entry = lhs_to_groups.entry(lhs.clone()).or_insert_with(|| vec!());
                        entry.push(rhs.clone());
                    }
                }
            }
        }
        return Self{groups, lhs_to_groups}
    }

    pub fn len(&self) -> usize {
        return self.groups.len();
    }

    pub fn mutate(&self, buff: &mut DataBuff, dist:  &Distributions) -> bool {
        if let Some(rhs) = self.sample_rhs(buff, dist) {
            if dist.gen(){  
                buff.copy_from(&rhs, 0);
                return dist.gen::<bool>();
            }
        }
        if let Some(entry) = self.sample_entry(buff,dist){
            match entry{
                DictEntry::Replace(lhs, rhs) => {
                    if let Some(pos) = self.find_pos(buff, &lhs, dist){
                        buff.copy_from(&rhs, pos);
                    }
                    return dist.gen::<bool>();
                }
            }
        }
        return true;
    }

    pub fn sample_rhs(&self, buff: &DataBuff, dist: &Distributions) -> Option<&Vec<u8>> {
        if self.lhs_to_groups.contains_key(buff.as_slice()) {
            let opts = &self.lhs_to_groups[buff.as_slice()];
            assert!(opts.len() > 0);
            return Some(&opts[dist.gen_range(0, opts.len())]);
        }
        return None
    }

    pub fn sample_entry(&self, buff: &DataBuff, dist: &Distributions) -> Option<&DictEntry> {
        if self.groups.len() == 0 {return None}
        let group = &self.groups[dist.gen_range(0,self.groups.len())];
        return Some(&group[dist.gen_range(0,group.len())]);
    }

    pub fn find_pos(&self, buff: &DataBuff, lhs: &[u8], dist: &Distributions) -> Option<usize>{
        let mut offsets = vec!();
        for (i,win) in buff.as_slice().windows(lhs.len()).enumerate() {
            if win == lhs {
                offsets.push(i);
            }
        }
        if offsets.len() > 0 {
            return Some(offsets[dist.gen_range(0,offsets.len())]);
        }
        if buff.len() >= lhs.len(){
            return Some(dist.gen_range(0,buff.len()-lhs.len()+1));
        }
        return None
    }
}

