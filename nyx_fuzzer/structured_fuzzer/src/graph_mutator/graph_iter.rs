use crate::graph_mutator::atomic_data::AtomicSize;
use crate::graph_mutator::newtypes::{ConnectorID, NodeTypeID, ValueTypeID};
use crate::graph_mutator::spec::{GraphSpec, NodeSpec};

pub struct GraphNode<'a> {
    pub id: NodeTypeID,
    pub op_i: usize,
    pub ops: &'a [u16],
    pub data: &'a [u8],
    pub spec: &'a GraphSpec,
}

impl<'a> GraphNode<'a> {
    pub fn iter_ops(&'a self) -> OpIter<'a> {
        return OpIter::new(self.ops, self.spec);
    }
}

pub struct NodeIter<'a> {
    graph_spec: &'a GraphSpec,
    op_i: usize,
    data_i: usize,
    op_frag: &'a [u16],
    data_frag: &'a [u8],
}

impl<'a> NodeIter<'a> {
    pub fn new(op_frag: &'a [u16], data_frag: &'a [u8], graph_spec: &'a GraphSpec) -> Self {
        return Self {
            op_frag,
            data_frag,
            op_i: 0,
            data_i: 0,
            graph_spec,
        };
    }
}

impl<'a> Iterator for NodeIter<'a> {
    type Item = GraphNode<'a>;

    fn next(&mut self) -> Option<GraphNode<'a>> {
        if self.op_i >= self.op_frag.len() {
            return None;
        }

        let id = NodeTypeID::new(self.op_frag[self.op_i]);
        let op_i = self.op_i;
        let n_type = self
            .graph_spec
            .get_node(id)
            .expect("invalid graph, couldn't get node type");
        let ops_len = 1 + n_type.size();
        let ops = &self.op_frag[self.op_i..self.op_i + ops_len];
        self.op_i += ops_len;

        let d_type = n_type.data.map(|did| {
            self.graph_spec
                .get_data(did)
                .expect("invalid spec, couldn't get data type")
        });
        let data_len = match d_type.map(|d| d.atomic_type.size()) {
            None => 0,
            Some(AtomicSize::Fixed(x)) => x,
            Some(AtomicSize::Dynamic()) => {
                let size = self.data_frag[self.data_i] as usize
                    | ((self.data_frag[self.data_i + 1] as usize) << 8);
                size + 2
            }
        };
        (
            "self.data_i {}, data_len {}, self.data_frag.len() {}",
            self.data_i,
            data_len,
            self.data_frag.len(),
        );
        assert!(self.data_i + data_len <= self.data_frag.len());
        let data = &self.data_frag[self.data_i..self.data_i + data_len];
        self.data_i += data_len;
        return Some(GraphNode {
            id,
            op_i,
            ops,
            data,
            spec: &self.graph_spec,
        });
    }
}

#[derive(Eq, PartialEq, Debug, Clone, Hash)]
pub enum GraphOp {
    Get(ValueTypeID, ConnectorID),
    Pass(ValueTypeID, ConnectorID),
    Set(ValueTypeID, ConnectorID),
    Node(NodeTypeID),
}

impl GraphOp {
    pub fn node(&self) -> Option<NodeTypeID> {
        if let GraphOp::Node(id) = self {
            return Some(*id);
        }
        return None;
    }
}

pub struct OpIter<'a> {
    frag: &'a [u16],
    graph_spec: &'a GraphSpec,
    i: usize,
    node_spec: Option<NodeTypeID>,
    inputs: usize,
    passthroughs: usize,
    outputs: usize,
}

impl<'a> OpIter<'a> {
    pub fn new(frag: &'a [u16], graph_spec: &'a GraphSpec) -> Self {
        return Self {
            frag,
            i: 0,
            graph_spec,
            node_spec: None,
            inputs: 0,
            passthroughs: 0,
            outputs: 0,
        };
    }

    fn next_op(&mut self) -> GraphOp {
        if self.inputs > 0 {
            let node = self.get_node();
            let inps = &node.inputs;
            let value_id = inps[inps.len() - self.inputs];
            self.inputs -= 1;
            return GraphOp::Get(value_id, ConnectorID::new(self.frag[self.i]));
        }
        if self.passthroughs > 0 {
            let node = self.get_node();
            let pass = &node.passthroughs;
            let value_id = pass[pass.len() - self.passthroughs];
            self.passthroughs -= 1;
            return GraphOp::Pass(value_id, ConnectorID::new(self.frag[self.i]));
        }
        if self.outputs > 0 {
            let node = self.get_node();
            let outs = &node.outputs;
            let value_id = outs[outs.len() - self.outputs];
            self.outputs -= 1;
            return GraphOp::Set(value_id, ConnectorID::new(self.frag[self.i]));
        }
        return GraphOp::Node(NodeTypeID::new(self.frag[self.i]));
    }

    fn get_node(&self) -> &NodeSpec {
        return self.graph_spec.get_node(self.node_spec.unwrap()).unwrap();
    }

    fn set_node(&mut self, n: NodeTypeID) {
        self.node_spec = Some(n);
        let node = self.graph_spec.get_node(n).unwrap();
        self.inputs = node.inputs.len();
        self.passthroughs = node.passthroughs.len();
        self.outputs = node.outputs.len();
    }
}

impl<'a> Iterator for OpIter<'a> {
    type Item = GraphOp;

    fn next(&mut self) -> Option<GraphOp> {
        if self.i < self.frag.len() {
            let op = self.next_op();
            if let GraphOp::Node(nt) = op {
                let node = self.graph_spec.get_node(nt).unwrap();
                self.set_node(node.id);
            }
            self.i += 1;
            return Some(op);
        }
        return None;
    }
}
