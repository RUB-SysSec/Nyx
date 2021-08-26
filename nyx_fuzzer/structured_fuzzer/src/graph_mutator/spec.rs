use crate::graph_mutator::atomic_data::AtomicDataType;
use crate::graph_mutator::newtypes::{AtomicTypeID, GraphError, NodeTypeID, ValueTypeID};
use std::collections::HashMap;
use std::sync::Arc;

#[derive(Default,Clone)]
pub struct GraphSpec {
    pub checksum: u64,
    pub node_specs: Vec<NodeSpec>,
    pub value_specs: Vec<ValueSpec>,
    pub data_specs: Vec<AtomicSpec>,

    max_data: usize,
    max_ops: usize,
}

impl GraphSpec {
    pub fn new() -> Self {
        return Self {
            checksum: 0,
            node_specs: Vec::new(),
            value_specs: Vec::new(),
            data_specs: Vec::new(),
            max_data: 0,
            max_ops: 0,
        };
    }

    pub fn biggest_data(&self) -> usize { return self.max_data; }
    pub fn biggest_ops(&self) -> usize { return self.max_ops; }

    pub fn get_node(&self, n: NodeTypeID) -> Result<&NodeSpec, GraphError> {
        return self
            .node_specs
            .get(n.as_usize())
            .ok_or(GraphError::UnknownNodeType(n));
    }

    pub fn get_value(&self, v: ValueTypeID) -> Result<&ValueSpec, GraphError> {
        return self
            .value_specs
            .get(v.as_usize())
            .ok_or(GraphError::UnknownValueType(v));
    }

    pub fn get_data(&self, v: AtomicTypeID) -> Result<&AtomicSpec, GraphError> {
        return self
            .data_specs
            .get(v.as_usize())
            .ok_or(GraphError::UnknownDataType(v));
    }

    pub fn get_node_size(&self, n: NodeTypeID) -> Result<usize, GraphError> {
        return Ok(self.get_node(n)?.size());
    }

    pub fn value_type(&mut self, name: &str) -> ValueTypeID {
        assert!(self.value_specs.len() < std::u16::MAX as usize);
        let new_id = ValueTypeID::new(self.value_specs.len() as u16);
        self.value_specs.push(ValueSpec::new(name, new_id));
        return new_id;
    }

    pub fn data_type(&mut self, name: &str, atom: Arc<dyn AtomicDataType+Send+Sync>) -> AtomicTypeID {
        let new_id = AtomicTypeID::new(self.data_specs.len());
        if self.max_data < atom.min_data_size() {self.max_data = atom.min_data_size(); }
        let spec = AtomicSpec::new(name, new_id, atom);
        self.data_specs.push(spec);
        return new_id;
    }

    pub fn node_type(
        &mut self,
        name: &str,
        data: Option<AtomicTypeID>,
        inputs: Vec<ValueTypeID>,
        passthrough: Vec<ValueTypeID>,
        outputs: Vec<ValueTypeID>,
    ) -> NodeTypeID {
        assert!(self.node_specs.len() < std::u16::MAX as usize);
        let new_id = NodeTypeID::new(self.node_specs.len() as u16);
        let ops_len = inputs.len() + passthrough.len() + outputs.len()+1;
        if self.max_ops < ops_len {self.max_ops = ops_len;}
        let spec = NodeSpec::new(name, new_id, data, inputs, passthrough, outputs);
        self.node_specs.push(spec);
        return new_id;
    }

    pub fn node_data_inspect(&self,n: NodeTypeID, data:&[u8]) -> String{
        let node = self.get_node(n).unwrap();
        if let Some(atom_id) = node.data {
            let atom = self.get_data(atom_id).unwrap();
            return format!(" {}:{}", atom.name,atom.atomic_type.data_inspect(data, self) );
        }
        return "".to_string();
    }
}

#[derive(Clone)]
pub struct AtomicSpec {
    pub name: String,
    pub id: AtomicTypeID,
    pub atomic_type: Arc<dyn AtomicDataType+Send+Sync>,
}

impl AtomicSpec {
    pub fn new(name: &str, id: AtomicTypeID, atomic_type: Arc<dyn AtomicDataType+Send+Sync>) -> Self {
        return AtomicSpec {
            name: name.to_string(),
            id,
            atomic_type,
        };
    }
}

#[derive(Clone)]
pub struct NodeSpec {
    pub name: String,
    pub id: NodeTypeID,
    pub inputs: Vec<ValueTypeID>,
    pub outputs: Vec<ValueTypeID>,
    pub passthroughs: Vec<ValueTypeID>,
    pub required_values: HashMap<ValueTypeID, usize>,
    pub data: Option<AtomicTypeID>,
}

impl NodeSpec {
    fn new(
        name: &str,
        id: NodeTypeID,
        data: Option<AtomicTypeID>,
        inputs: Vec<ValueTypeID>,
        passthroughs: Vec<ValueTypeID>,
        outputs: Vec<ValueTypeID>,
    ) -> Self {
        let mut required_values = HashMap::new();

        for pass in passthroughs.iter() {
            if *required_values.entry(*pass).or_insert(0) == 0 {
                *required_values.entry(*pass).or_insert(0) = 1;
            }
        }
        for inp in inputs.iter() {
            *required_values.entry(*inp).or_insert(0) += 1;
        }

        return Self {
            name: name.to_string(),
            id,
            inputs,
            outputs,
            passthroughs,
            required_values,
            data,
        };
    }

    pub fn size(&self) -> usize {
        return self.inputs.len() + self.passthroughs.len() + self.outputs.len();
    }

    pub fn min_data_size(&self, spec: &GraphSpec) -> usize {
        return self
            .data
            .map(|d| spec.get_data(d).unwrap())
            .map(|spec| spec.atomic_type.min_data_size())
            .unwrap_or(0);
    }
}

#[derive(Clone)]
pub struct ValueSpec {
    pub id: ValueTypeID,
    pub name: String,
}

impl ValueSpec {
    pub fn new(name: &str, id: ValueTypeID) -> Self {
        return Self {
            name: name.to_string(),
            id,
        };
    }
}
