use crate::graph_mutator::graph_iter::{GraphNode, GraphOp, NodeIter, OpIter};
use crate::graph_mutator::newtypes::{DstVal, NodeTypeID, ValueTypeID, OpIndex, PortID, SrcVal};
use crate::graph_mutator::spec::GraphSpec;


use crate::itertools::Itertools;

use std::collections::HashMap;
use std::fs::File;
use std::io::prelude::*;

//Needs to be used as a wrapper over Graph Storage, as Graph Storage can't be made into an object due to the Sized constraint
pub trait GraphMutationTarget {
    fn append_op(&mut self, op: u16) -> Option<()>;
    fn append_data(&mut self, data: &[u8]) -> Option<&mut [u8]>;
    fn get_data(&mut self, size: usize) -> Option<&mut [u8]>;
    fn data_available(&self) -> usize;
    fn ops_available(&self) -> usize;
}

impl<T: GraphStorage> GraphMutationTarget for T {
    fn append_op(&mut self, op: u16) -> Option<()> {
        return self.append_op(op);
    }
    fn append_data(&mut self, data: &[u8]) -> Option<&mut [u8]> {
        return self.append_data(data);
    }
    fn get_data(&mut self, size: usize) -> Option<&mut [u8]> {
        return self.get_data(size);
    }
    fn data_available(&self) -> usize {
        return self.data_available();
    }
    fn ops_available(&self) -> usize {
        return self.ops_available();
    }
}

pub trait GraphStorage: Sized {
    fn clear(&mut self);
    fn append_op(&mut self, op: u16) -> Option<()>;
    fn append_data(&mut self, data: &[u8]) -> Option<&mut [u8]>;
    fn get_data(&mut self, size: usize) -> Option<&mut [u8]>;
    fn data_available(&self) -> usize;
    fn ops_available(&self) -> usize;
    fn can_append(&self, node: &GraphNode) -> bool;
    fn data_len(&self) -> usize;
    fn op_len(&self) -> usize;
    fn node_len(&self, spec:&GraphSpec) -> usize;
    fn is_empty(&self) -> bool {
        return self.op_len() == 0;
    }
    fn ops_as_slice(&self) -> &[u16];
    fn data_as_slice(&self) -> &[u8];

    fn as_vec_graph(&self) -> VecGraph {
        let mut ops = Vec::with_capacity(self.op_len());
        let mut data = Vec::with_capacity(self.data_len());
        ops.extend_from_slice(self.ops_as_slice());
        data.extend_from_slice(self.data_as_slice());
        let res = VecGraph::new(ops, data);
        return res;
    }

    fn copy_from(&mut self, graph: &VecGraph)  {
        self.clear();
        for op in graph.ops_as_slice(){
            self.append_op(*op);
        }
        self.append_data(graph.data_as_slice());
    }
    
    fn calc_edges(&self, spec: &GraphSpec) -> Vec<(SrcVal, DstVal)> {
        let mut res = vec![];
        let mut id_to_src = HashMap::new();
        let mut last_node = None;
        let mut last_out_port = PortID::new(0);
        let mut last_in_port = PortID::new(0);
        let mut last_pass_port = PortID::new(0);
        for (i, op) in self.op_iter(spec).enumerate() {
            match op {
                GraphOp::Node(_n_type_id) => {
                    last_node = Some(OpIndex::new(i));
                    last_in_port = PortID::new(0);
                    last_pass_port = PortID::new(0);
                    last_out_port = PortID::new(0);
                }
                GraphOp::Set(vt, id) => {
                    id_to_src.insert((vt, id), SrcVal::new(last_node.unwrap(), last_out_port));
                    last_out_port = last_out_port.next();
                }
                GraphOp::Get(vt, id) => {
                    res.push((
                        id_to_src.remove(&(vt, id)).unwrap(),
                        DstVal::new(last_node.unwrap(), last_in_port),
                    ));
                    last_in_port = last_in_port.next();
                }
                GraphOp::Pass(vt, id) => {
                    res.push((
                        id_to_src.get(&(vt, id)).cloned().unwrap(),
                        DstVal::new(last_node.unwrap(), last_pass_port),
                    ));
                    last_pass_port = last_pass_port.next();
                }
            }
        }
        return res;
    }

    fn write_to_file(&self, path: &str, spec: &GraphSpec) {
        use std::io::BufWriter;
        use std::io::prelude::*;
        let mut file = BufWriter::new(File::create(path).expect(&format!("couldn't open file to dump input {}",path)));
        file.write(&spec.checksum.to_le_bytes()).expect("couldn't write checksum");
        file.write(&(self.ops_as_slice().len() as u64).to_le_bytes()).expect("couldn't write graph op len");
        file.write(&(self.data_as_slice().len() as u64).to_le_bytes()).expect("couldn't write graph data len");
        file.write(&(5*8_u64).to_le_bytes()).expect("couldn't write graph op offset");
        file.write(&(5*8+(self.ops_as_slice().len() as u64)*2_u64).to_le_bytes()).expect("couldn't write graph data offset");
        for b in self.ops_as_slice().iter(){
            file.write(&b.to_le_bytes()).expect("couldn't write graph op");
        }
        file.write(self.data_as_slice()).expect("couldn't write graph data");
    }

    fn node_iter<'a>(&'a self, spec: &'a GraphSpec) -> NodeIter<'a> {
        return NodeIter::new(self.ops_as_slice(), self.data_as_slice(), spec);
    }
    fn op_iter<'a>(&'a self, spec: &'a GraphSpec) -> OpIter<'a> {
        return OpIter::new(self.ops_as_slice(), spec);
    }

    fn to_svg(&self, path: &str, spec: &GraphSpec) {
        use std::process::{Command, Stdio};
        let dot = self.to_dot(spec);
        let mut child = Command::new("dot")
            .stdout(Stdio::inherit())
            .stdin(Stdio::piped())
            .arg("-Tsvg")
            .arg("-o")
            .arg(path)
            .spawn()
            .expect("failed to execute dot");
        child
            .stdin
            .as_mut()
            .unwrap()
            .write(&dot.as_bytes())
            .expect("failed to write dot graph");
        child.wait().expect("failed to wait on dot");
    }

    fn to_png(&self, path: &str, spec: &GraphSpec) {
        use std::process::{Command, Stdio};
        let dot = self.to_dot(spec);
        let mut child = Command::new("dot")
            .stdout(Stdio::inherit())
            .stdin(Stdio::piped())
            .arg("-Tpng")
            .arg("-o")
            .arg(path)
            .spawn()
            .expect("failed to execute dot");
        child
            .stdin
            .as_mut()
            .unwrap()
            .write(&dot.as_bytes())
            .expect("failed to write dot graph");
        child.wait().expect("failed to wait on dot");
    }

    fn var_names(ops: &[u16], types: &[ValueTypeID], spec: &GraphSpec) -> String{
        return ops.iter().enumerate().map(|(i,id)| format!("v_{}_{}",spec.get_value(types[i]).unwrap().name, id)).intersperse(", ".to_string()).collect::<String>();
    }

    fn write_to_script_file(&self, path: &str, spec: &GraphSpec) {
        let mut file = File::create(path).unwrap();
        file.write_all(self.to_script(spec).as_bytes()).unwrap()
    }

    fn to_script(&self, spec: &GraphSpec) -> String {
        let mut res = "".to_string();
        for n in self.node_iter(spec){
            let node = n.spec.get_node(n.id).unwrap();
            let data = spec.node_data_inspect(n.id, n.data).replace("\\l","");
            let i = 1+node.inputs.len();
            let inputs = Self::var_names(&n.ops[1..i], &node.inputs, spec);
            let j = i + node.passthroughs.len();
            let borrows = Self::var_names(&n.ops[i..j], &node.passthroughs, spec);
            let outputs = Self::var_names(&n.ops[j..], &node.outputs, spec);
            if outputs.len() > 0 {
                res += &outputs;
                res += " = ";
            }
            res+= &format!("{}( inputs=[{}], borrows=[{}], data={})\n",node.name, inputs, borrows, data)
        }
        //println!("{}",res);
        return res;
    }

    fn write_to_dot_file(&self, path: &str, spec: &GraphSpec) {
        let mut file = File::create(path).unwrap();
        file.write_all(self.to_dot(spec).as_bytes()).unwrap()
    }

    fn to_dot(&self, spec: &GraphSpec) -> String {
        let mut res = "digraph{\n rankdir=\"LR\";\n { edge [style=invis weight=100];".to_string();
        let edges = self.calc_edges(spec);
        let mut join = "";
        for (_ntype, i) in self
            .op_iter(&spec)
            .enumerate()
            .filter_map(|(i, op)| op.node().map(|n| (n, i)))
        {
            res += &format!("{}n{}", join, i);
            join = "->";
        }
        res += "}\n";
        for node in self.node_iter(spec) {
            res += &format!(
                "n{} [label=\"{}{}\", shape=box];\n",
                node.op_i,
                spec.get_node(node.id).unwrap().name,
                spec.node_data_inspect(node.id, node.data),
            );
        }
        for (src, dst) in edges.iter() {
            let node_type = NodeTypeID::new(self.ops_as_slice()[src.id.as_usize()]);
            let value_type = spec.get_node(node_type).unwrap().outputs[src.port.as_usize()];
            let edge_type = &spec.get_value(value_type).unwrap().name;
            res += &format!(
                "n{} -> n{} [label=\"{}\"];\n",
                src.id.as_usize(),
                dst.id.as_usize(),
                edge_type
            );
        }
        res += "}";
        //println!("{}", res);
        return res;
    }
}

#[derive(Clone)]
pub struct VecGraph {
    ops: Vec<u16>,
    data: Vec<u8>,
}

impl VecGraph {

    pub fn new_with_size(op_len: usize, data_len: usize) -> Self{
        return Self::new(vec!(0; op_len), vec!(0; data_len));
    }

    pub fn new(ops: Vec<u16>, data: Vec<u8>) -> Self {
        return Self { ops, data,};
    }

    pub fn new_from_bin_file(path: &str, spec: &GraphSpec) -> VecGraph{

        use std::io::BufReader;
        use std::convert::TryInto;

        let mut f = BufReader::new(File::open(path).expect("youldn't open .bin file for reading"));
        let mut buffer = [0; 8];
        let n = f.read(&mut buffer[..]).expect("couldn't read checksum");
        assert_eq!(n,8);
        let checksum = u64::from_le_bytes(buffer.try_into().unwrap());
        assert_eq!(checksum, spec.checksum);

        let n = f.read(&mut buffer[..]).expect("couldn't read num_ops");
        assert_eq!(n,8);
        let num_ops = u64::from_le_bytes(buffer.try_into().unwrap());

        let n = f.read(&mut buffer[..]).expect("couldn't read num_data");
        assert_eq!(n,8);
        let num_data = u64::from_le_bytes(buffer.try_into().unwrap());

        let n = f.read(&mut buffer[..]).expect("couldn't read op_offset");
        assert_eq!(n,8);
        let op_offset = u64::from_le_bytes(buffer.try_into().unwrap());

        let n = f.read(&mut buffer[..]).expect("couldn't read data_offset");
        assert_eq!(n,8);
        let data_offset = u64::from_le_bytes(buffer.try_into().unwrap());

        f.seek(std::io::SeekFrom::Start(op_offset)).unwrap();

        let mut res = VecGraph::empty();
        for _i in 0..num_ops{
            let n = f.read(&mut buffer[..2]).unwrap();
            assert_eq!(n,2);
            let op = u16::from_le_bytes(buffer[..2].try_into().unwrap());
            res.ops.push(op);
        }

        f.seek(std::io::SeekFrom::Start(data_offset)).unwrap();
        res.data = vec!(0; num_data as usize);
        f.read_exact(&mut res.data[..]).unwrap();
        return res;
    }

    pub fn empty() -> Self {
        return Self::new(vec![], vec![]);
    }

    pub fn as_ref_graph<'a>(&'a mut self, ops_i: &'a mut usize, data_i: &'a mut usize) -> RefGraph<'a>{
        return RefGraph::new(&mut self.ops[..], &mut self.data[..], ops_i, data_i);
    }
}

impl GraphStorage for VecGraph {
    fn clear(&mut self) {
        self.ops.clear();
        self.data.clear();
    }
    fn append_op(&mut self, op: u16) -> Option<()> {
        self.ops.push(op);
        return Some(());
    }

    fn get_data(&mut self, size: usize) -> Option<&mut [u8]> {
        let len = self.data.len();
        self.data.resize(len + size, 0);
        return Some(&mut self.data[len..]);
    }

    fn append_data(&mut self, data: &[u8]) -> Option<&mut [u8]> {
        self.data.extend_from_slice(data);
        let len = self.data.len();
        return Some(&mut self.data[len - data.len()..]);
    }

    fn data_available(&self) -> usize {
        return 0xffffffff;
    }

    fn ops_available(&self) -> usize {
        return 0xffffffff;
    }

    fn can_append(&self, _n: &GraphNode) -> bool {
        return true;
    }

    fn op_len(&self) -> usize {
        return self.ops.len();
    }
    
    fn node_len(&self, spec:&GraphSpec) -> usize{
        return self.node_iter(spec).count();
    }

    fn data_len(&self) -> usize {
        return self.data.len();
    }
    fn ops_as_slice(&self) -> &[u16] {
        return &self.ops[..];
    }
    fn data_as_slice(&self) -> &[u8] {
        return &self.data[..];
    }
}

pub struct RefGraph<'a> {
    ops: &'a mut [u16],
    ops_i: &'a mut usize,
    data: &'a mut [u8],
    data_i: &'a mut usize,
}

impl<'a> RefGraph<'a> {
    pub fn new(
        ops: &'a mut [u16],
        data: &'a mut [u8],
        ops_i: &'a mut usize,
        data_i: &'a mut usize,
    ) -> Self {
        return Self {
            ops,
            ops_i,
            data,
            data_i,
        };
    }

    pub fn new_from_slice(payload: &mut [u8], checksum: u64) -> Self {
        let header_len = std::mem::size_of::<u64>() * 5;
        let data_len = payload.len() - header_len;
        assert_eq!(data_len % 8, 0);
        let buff_size = data_len / 2;

        unsafe {
            let ptr = payload.as_mut_ptr();
            assert_eq!((ptr as usize) % std::mem::align_of::<u64>(), 0);
            let checksum_ptr = (ptr as *mut u64).add(0).as_mut().unwrap();
            let ops_i_ptr = (ptr as *mut usize).add(1).as_mut().unwrap();
            let data_i_ptr = (ptr as *mut usize).add(2).as_mut().unwrap();
            assert_eq!(std::mem::size_of::<u64>(), std::mem::size_of::<usize>());
            let graph_offset_ptr = (ptr as *mut u64).add(3).as_mut().unwrap();
            let data_offset_ptr = (ptr as *mut u64).add(4).as_mut().unwrap();

            assert_eq!(std::mem::size_of::<usize>(), std::mem::size_of::<u64>());
            *checksum_ptr = checksum;
            *graph_offset_ptr = header_len as u64;
            *data_offset_ptr = (header_len + buff_size) as u64;

            let op_ptr = ptr.add(*graph_offset_ptr as usize) as *mut u16;
            let struct_buff = std::slice::from_raw_parts_mut(op_ptr, buff_size / 2);

            let data_ptr = ptr.add(*data_offset_ptr as usize) as *mut u8;
            let data_buff = std::slice::from_raw_parts_mut(data_ptr, buff_size);

            assert_eq!(*data_offset_ptr as usize + buff_size, payload.len());

            return RefGraph::new(struct_buff, data_buff, ops_i_ptr, data_i_ptr);
        }
    }
}

impl<'a> GraphStorage for RefGraph<'a> {
    fn clear(&mut self) {
        *self.ops_i = 0;
        *self.data_i = 0;
    }

    fn append_op(&mut self, op: u16) -> Option<()> {
        if (*self.ops_i as usize) < self.ops.len() {
            self.ops[*self.ops_i as usize] = op;
            *self.ops_i += 1;
            return Some(());
        }
        return None;
    }

    fn get_data(&mut self, size: usize) -> Option<&mut [u8]> {
        if *self.data_i + size <= self.data.len() {
            let range = *self.data_i..*self.data_i + size;
            *self.data_i += size;
            return Some(&mut self.data[range]);
        }
        return None;
    }

    fn append_data(&mut self, data: &[u8]) -> Option<&mut [u8]> {
        if *self.data_i + data.len() <= self.data.len() {
            let range = *self.data_i..(*self.data_i + data.len());
            *self.data_i += data.len();
            let buf = &mut self.data[range];
            buf.copy_from_slice(data);
            return Some(buf);
        }
        return None;
    }

    fn data_available(&self) -> usize {
        return self.data.len() - *self.data_i;
    }
    fn ops_available(&self) -> usize {
        return self.ops.len() - *self.ops_i;
    }

    fn can_append(&self, n: &GraphNode) -> bool {
        return *self.data_i + n.data.len() < self.data.len();
    }

    fn op_len(&self) -> usize {
        return *self.ops_i;
    }

    fn node_len(&self, spec:&GraphSpec) -> usize{
        return self.node_iter(spec).count();
    }

    fn data_len(&self) -> usize {
        return *self.data_i;
    }
    fn ops_as_slice(&self) -> &[u16] {
        return &self.ops[..*self.ops_i];
    }
    fn data_as_slice(&self) -> &[u8] {
        return &self.data[..*self.data_i];
    }
}
