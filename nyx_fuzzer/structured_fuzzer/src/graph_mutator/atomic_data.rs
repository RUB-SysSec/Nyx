use crate::graph_mutator::graph_storage::GraphMutationTarget;
use crate::graph_mutator::newtypes::AtomicTypeID;
use crate::graph_mutator::spec::GraphSpec;
use crate::custom_dict::CustomDict;

use crate::data_buff::DataBuff;
use crate::primitive_mutator::mutator::PrimitiveMutator;
use crate::graph_mutator::generators::IntGenerator;
use crate::random::distributions::Distributions;

#[derive(Debug)]
pub enum AtomicSize {
    Fixed(usize),
    Dynamic(),
}

impl AtomicSize {
    pub fn is_fixed(&self) -> bool {
        return match self {
            AtomicSize::Fixed(_) => true,
            _ => false,
        };
    }

    pub fn as_usize(&self) -> usize {
        return match self {
            AtomicSize::Fixed(x) => *x,
            _ => panic!("can't call as_usize on dynamic sized atomic data"),
        };
    }

    pub fn min_data_size(&self) -> usize {
        match self {
            AtomicSize::Fixed(x) => *x,
            AtomicSize::Dynamic() => 2,
        }
    }
}

//TODO try moving the S: GraphStorage into the append_generate / append_mutated
pub trait AtomicDataType {
    fn size(&self) -> AtomicSize;
    fn append_generate(
        &self,
        storage: &mut dyn GraphMutationTarget,
        spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    );
    fn append_mutated<'a>(
        &'a self,
        _data: &[u8],
        dict: &CustomDict,
        storage: &mut dyn GraphMutationTarget,
        spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    );
    fn min_data_size(&self) -> usize {
        return self.size().min_data_size();
    }

    fn data_inspect(&self, data:&[u8], spec: &GraphSpec) -> String;
}

pub struct DataInt {
    size: usize,
    generators: Vec<IntGenerator>,
}

impl DataInt {
    pub fn new(size: usize, generators: Vec<IntGenerator>) -> Self {
        match size {
            1 | 2 | 4 | 8 => return Self { size, generators },
            _ => panic!("invalid integer size {}", size),
        }
    }

    fn generate_u64(&self, mutator: &PrimitiveMutator, dist: &Distributions) -> (u64,bool){
        if self.generators.len() == 0 { return (dist.gen::<u64>(), true) }
        let gen = &self.generators[dist.gen_range(0,self.generators.len())];
        return gen.generate( dist );
    }
    
    fn read_from_data(&self, data: &[u8]) -> u64 {
        use std::convert::TryInto;
        match self.size {
            1 => return u8::from_le_bytes(data.try_into().unwrap()) as u64,
            2 => return u16::from_le_bytes(data.try_into().unwrap()) as u64,
            4 => return u32::from_le_bytes(data.try_into().unwrap()) as u64,
            8 => return u64::from_le_bytes(data.try_into().unwrap()) as u64,
            _ => panic!("invalid integer size {}", self.size),
        }
    }
}

impl AtomicDataType for DataInt {
    fn append_generate(
        &self,
        storage: &mut dyn GraphMutationTarget,
        _spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        let (int,mutate_further) = self.generate_u64(mutator, dist);
        let data = storage.append_data(&int.to_le_bytes()[0..self.size]).unwrap();
        if mutate_further {
            let mut buf = DataBuff::new(data, self.size);
            for _ in (0..dist.gen::<usize>() % 4){
                mutator.mutate(&mut buf, None, dist);
            }
        }
    }
    fn size(&self) -> AtomicSize {
        return AtomicSize::Fixed(self.size);
    }
    fn append_mutated<'a>(
        &'a self,
        data: &[u8],
        dict: &CustomDict,
        storage: &mut dyn GraphMutationTarget,
        _spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        let copy = storage.append_data(data).unwrap();
        let mut buf = DataBuff::new(copy, copy.len());
        mutator.mutate(&mut buf, Some(dict), dist);
    }
    
    fn data_inspect(&self, data:&[u8], _spec: &GraphSpec) -> String{ 
        match self.size{
            1 => return format!("0x{:02x}", self.read_from_data(data) ),
            2 => return format!("0x{:04x}", self.read_from_data(data) ),
            4 => return format!("0x{:08x}", self.read_from_data(data) ),
            8 => return format!("0x{:016x}", self.read_from_data(data) ),
            _ => panic!("invalid integer size {}", self.size),
        }
    }
}

pub struct DataVec {
    size_range: (usize, usize),
    primitive_size: usize,
    dtype: AtomicTypeID,
}

impl DataVec {
    pub fn new(size_range: (usize, usize), dtype: AtomicTypeID, spec: &GraphSpec) -> Self {
        let primitive_size = spec.get_data(dtype).unwrap().atomic_type.size().as_usize();
        return Self {
            size_range,
            dtype: dtype,
            primitive_size,
        };
    }
    
}

impl AtomicDataType for DataVec {
    fn append_generate(
        &self,
        storage: &mut dyn GraphMutationTarget,
        _spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        let num_elems = mutator.gen_num_array_elems(
            self.primitive_size,
            self.size_range.0,
            self.size_range.1,
            storage.data_available() - 2,
            dist
        );
        let size = num_elems * self.primitive_size;
        assert!(size + 2 <= storage.data_available());
        let data = storage.get_data(size + 2).unwrap();
        data[0] = (size & 0xff) as u8;
        data[1] = ((size >> 8) & 0xff) as u8;
        assert!(data.len() == size + 2);
        if size > 0 {
            mutator.mutate(&mut DataBuff::new(&mut data[2..], size), None, dist);
        }
    }
    fn size(&self) -> AtomicSize {
        return AtomicSize::Dynamic();
    }
    fn append_mutated<'a>(
        &'a self,
        data: &[u8],
        dict: &CustomDict,
        storage: &mut dyn GraphMutationTarget,
        _spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        assert_eq!(data[0] as usize + ((data[1] as usize) << 8) + 2, data.len());
        let copy = storage.append_data(data).unwrap();
        let len = copy.len() - 2;
        mutator.mutate(&mut DataBuff::new(&mut copy[2..], len), Some(dict), dist);
    }

    fn data_inspect(&self, data:&[u8], spec: &GraphSpec) -> String{ 
        let mut res = "[".to_string();
        assert_eq!(data.len()%self.primitive_size, 0);
        let atom = spec.get_data(self.dtype).unwrap();
        for i in 0..data.len()/self.primitive_size{
            res+= &format!("{}, ",atom.atomic_type.data_inspect(&data[i..i+self.primitive_size], spec));
        }
        return res+"]";
    }
}

pub struct DataStruct {
    size: usize,
    fields: Vec<(String, AtomicTypeID)>,
}

impl DataStruct {
    pub fn new(fields: Vec<(String, AtomicTypeID)>, spec: &GraphSpec) -> Self {
        let size = fields
            .iter()
            .map(|(_, id)| {
                spec.get_data(*id)
                    .expect("invalid field type in struct")
                    .atomic_type
                    .size()
                    .as_usize()
            })
            .sum();
        return Self {
            size,
            fields: fields,
        };
    }
}

impl AtomicDataType for DataStruct {
    fn append_generate(
        &self,
        storage: &mut dyn GraphMutationTarget,
        spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        for (_,dt) in self.fields.iter(){
            spec.get_data(*dt).unwrap().atomic_type.append_generate(storage, spec, mutator, dist);
        }
        //let mut data = storage.get_data(self.size).unwrap();
        //let len = data.len();
        //mutator.mutate(&mut DataBuff::new(&mut data, len));
    }
    fn size(&self) -> AtomicSize {
        return AtomicSize::Fixed(self.size);
    }
    fn append_mutated<'a>(
        &'a self,
        data: &[u8],
        dict: &CustomDict,
        storage: &mut dyn GraphMutationTarget,
        _spec: &GraphSpec,
        mutator: &PrimitiveMutator,
        dist: &Distributions
    ) {
        let mut copy = storage.append_data(data).unwrap();
        let len = copy.len();
        mutator.mutate(&mut DataBuff::new(&mut copy, len), Some(dict), dist);
    }
    fn data_inspect(&self, data:&[u8], spec: &GraphSpec) -> String{ 
        let mut res = "{\\l".to_string();
        let mut i = 0;
        for (name, atomic_id) in self.fields.iter() {
            let atom = spec.get_data(*atomic_id).unwrap();
            let size = atom.atomic_type.size().as_usize();
            res+= &format!("{}: {},\\l ",name, atom.atomic_type.data_inspect(&data[i..i+size], spec));
            i+=size;
        }
        return res+"}";
    }
}
