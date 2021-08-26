#![allow(clippy::needless_return)]
extern crate rand;
extern crate rand_core;

extern crate serde;
#[macro_use]
extern crate serde_derive;
extern crate rmp_serde as rmps;
extern crate itertools;

pub mod data_buff;
pub mod graph_mutator;
pub mod mutator;
pub mod primitive_mutator;
pub mod random;
pub mod custom_dict;

pub use graph_mutator::atomic_data;
pub use graph_mutator::graph_builder::GraphBuilder;
pub use graph_mutator::graph_storage::{GraphStorage, VecGraph};
pub use graph_mutator::newtypes::{AtomicTypeID, DstVal, OpIndex, PortID, SrcVal};
pub use graph_mutator::spec::{GraphSpec, NodeSpec, ValueSpec};


#[cfg(test)]
mod graph_spec {
    use super::graph_mutator::graph_iter::GraphOp;
    use super::graph_mutator::newtypes::{ConnectorID, NodeTypeID, ValueTypeID};
    use super::*;
    use std::rc::Rc;
    use std::sync::Arc;
    use super::random::distributions::Distributions;

    fn build_spec() -> GraphSpec {
        let mut gs = GraphSpec::new();
        let va = gs.value_type("a");
        assert_eq!(va, ValueTypeID::new(0));
        let vb = gs.value_type("b");
        assert_eq!(vb, ValueTypeID::new(1));

        let data = None;
        let na = gs.node_type("na", data, vec![], vec![], vec![va]);
        assert_eq!(na, NodeTypeID::new(0));
        let nb = gs.node_type("nb", data, vec![va], vec![], vec![vb]);
        assert_eq!(nb, NodeTypeID::new(1));
        let nc = gs.node_type("nc", data, vec![va, vb], vec![], vec![]);
        assert_eq!(nc, NodeTypeID::new(2));
        let nd = gs.node_type("nd", data, vec![], vec![va], vec![]);
        assert_eq!(nd, NodeTypeID::new(3));
        return gs;
    }

    fn build_frag() -> Vec<u16> {
        return vec![
            0,    //na
            1337, //connection id for set(va)
            0,    //na
            42,   //connection if for set(va)
            1,    //nb
            1337, //connection id get(va)
            43,   //connection id set(vb)
            2,    //nc
            42,   // connection id get(va)
            43,   // connection id get(vb)
        ];
    }

    fn builder() -> (GraphBuilder, VecGraph) {
        let spec = build_spec();
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(spec));
        let mut storage = VecGraph::new(vec![], vec![]);
        gb.start(&mut storage);
        gb.append_slice(&build_frag(), &mut storage, &dist);
        return (gb, storage);
    }

    #[test]
    fn test_spec() {
        let (gb, st) = builder();
        let graph = gb.finalize(&st);

        assert_eq!(
            graph.ops_as_slice(),
            &vec!(
                0, //na
                1, //connection id for set(va)
                0, //na
                2, //connection if for set(va)
                1, //nb
                1, //connection id get(va)
                1, //connection id set(vb)
                2, //nc
                2, // connection id get(va)
                1, // connection id get(vb)
            )[..]
        );

        let ops = graph.op_iter(&gb.spec).collect::<Vec<_>>();
        assert_eq!(
            ops,
            vec!(
                GraphOp::Node(NodeTypeID::new(0)), //0
                GraphOp::Set(ValueTypeID::new(0), ConnectorID::new(1)),
                GraphOp::Node(NodeTypeID::new(0)), //2
                GraphOp::Set(ValueTypeID::new(0), ConnectorID::new(2)),
                GraphOp::Node(NodeTypeID::new(1)), //4
                GraphOp::Get(ValueTypeID::new(0), ConnectorID::new(1)),
                GraphOp::Set(ValueTypeID::new(1), ConnectorID::new(1)),
                GraphOp::Node(NodeTypeID::new(2)), //7
                GraphOp::Get(ValueTypeID::new(0), ConnectorID::new(2)),
                GraphOp::Get(ValueTypeID::new(1), ConnectorID::new(1)),
            )
        )
    }

    #[test]
    fn test_edges() {
        let (gb, st) = builder();
        let graph = gb.finalize(&st);
        let edges = graph.calc_edges(&gb.spec);
        assert_eq!(
            edges,
            vec!(
                (
                    SrcVal::new(OpIndex::new(0), PortID::new(0)),
                    DstVal::new(OpIndex::new(4), PortID::new(0))
                ),
                (
                    SrcVal::new(OpIndex::new(2), PortID::new(0)),
                    DstVal::new(OpIndex::new(7), PortID::new(0))
                ),
                (
                    SrcVal::new(OpIndex::new(4), PortID::new(0)),
                    DstVal::new(OpIndex::new(7), PortID::new(1))
                ),
            )
        )
    }

    fn build_redundant_graph(gb: &mut GraphBuilder, st: &mut VecGraph, dist: &Distributions) -> VecGraph {
        let frag = vec![
            0,    //na
            1337, //connection id for set(va)
            3,    //nd
            1337, //connection id for pass(va)
            3,    //nd
            1337, //connection id for pass(va)
            1,    //nb
            1337, //connection id get(va)
            43,   //connection id set(vb)
        ];
        let graph = VecGraph::new(frag, vec![]);
        gb.start(st);
        gb.append_slice(&graph.ops_as_slice(), st, dist);
        return gb.finalize(st);
    }

    #[test]
    fn test_drop_at() {
        let spec = build_spec();
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(spec));
        let mut st = VecGraph::new(vec![], vec![]);
        let graph = build_redundant_graph(&mut gb, &mut st, &dist);
        gb.drop_node_at(&graph, 1, &mut st, &dist);
        assert_eq!(
            gb.finalize(&st).ops_as_slice(),
            &vec!(0, 1, 3, 1, 1, 1, 1)[..]
        );
        gb.drop_node_at(&graph, 2, &mut st, &dist);
        assert_eq!(
            gb.finalize(&st).ops_as_slice(),
            &vec!(0, 1, 3, 1, 1, 1, 1)[..]
        );
        gb.drop_node_at(&graph, 3, &mut st, &dist);
        assert_eq!(gb.finalize(&st).ops_as_slice(), &vec!(0, 1, 3, 1, 3, 1)[..]);
        gb.drop_node_at(&graph, 0, &mut st, &dist);
    }

    #[test]
    fn test_minimize() {
        let spec = build_spec();
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(spec));
        let mut st = VecGraph::new(vec![], vec![]);
        let graph = build_redundant_graph(&mut gb, &mut st, &dist);
        println!("FNORD {}", graph.to_dot(&gb.spec));
        let min = gb.minimize(&graph, &mut st, &dist, |gr: &VecGraph, spec| {
            println!("test {}", gr.to_dot(spec));
            gr.to_dot(spec).contains("nb")
        });
        assert_eq!(min.ops_as_slice(), &vec!(0, 1, 1, 1, 1)[..]);
    }

    #[test]
    fn test_fuzz_minimize() {
        let mut gs = GraphSpec::new();

        let t_path = gs.value_type("path");
        let t_fd = gs.value_type("fd");
        let t_mmap_buffer = gs.value_type("mmap_buffer");

        let dt = Some(gs.data_type("none", Arc::new(atomic_data::DataInt::new(1, vec!()))));

        let _n_path = gs.node_type("path", dt, vec![], vec![], vec![t_path]);
        let _n_open = gs.node_type("open", dt, vec![], vec![t_path], vec![t_fd]);
        let _n_mmap = gs.node_type("mmap", dt, vec![], vec![t_fd], vec![t_mmap_buffer]);
        let _n_stdout = gs.node_type("fd:0", dt, vec![], vec![], vec![t_fd]);
        let _n_dup2 = gs.node_type("dup2", dt, vec![], vec![t_fd, t_fd], vec![]);
        let _n_close = gs.node_type("close", dt, vec![t_fd], vec![], vec![]);
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(gs));
        let mut st = VecGraph::empty();
        let mutator = primitive_mutator::mutator::PrimitiveMutator::new();
        for _ in 0..10 {
            gb.start(&mut st);
            gb.append_random(50, &mutator, &mut st,&dist).unwrap();
            let graph = gb.finalize(&st);
            if graph.to_dot(&gb.spec).contains("dup2") {
                let min = gb.minimize(&graph, &mut st, &dist, |gr: &VecGraph, spec| {
                    let res = gr.to_dot(spec).contains("dup2");
                    res
                });
                assert!(min.op_len() <= 8); //TODO min.len() should be < 8, fix minimization!
            }
        }
    }

    fn example_io_spec() -> GraphSpec {
        let mut gs = GraphSpec::new();

        let t_path = gs.value_type("path");
        let t_fd = gs.value_type("fd");
        let t_mmap_buffer = gs.value_type("mmap_buffer");

        let dt = None;

        let _n_path = gs.node_type("path", dt, vec![], vec![], vec![t_path]);
        let _n_open = gs.node_type("open", dt, vec![], vec![t_path], vec![t_fd]);
        let _n_mmap = gs.node_type("mmap", dt, vec![], vec![t_fd], vec![t_mmap_buffer]);
        let _n_stdout = gs.node_type("fd:0", dt, vec![], vec![], vec![t_fd]);
        let _n_dup2 = gs.node_type("dup2", dt, vec![], vec![t_fd, t_fd], vec![]);
        let _n_close = gs.node_type("close", dt, vec![t_fd], vec![], vec![]);
        gs
    }

    #[test]
    pub fn test_drop_1() {
        let spec = example_io_spec();
        let data = vec![0, 1, 3, 1, 1, 1, 2, 4, 1, 2];
        let graph = VecGraph::new(data, vec![]);
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(spec));
        let mut st = VecGraph::empty();
        gb.drop_node_at(&graph, 2, &mut st, &dist);
    }

    #[test]
    fn test_to_dot() {
        let (gb, st) = builder();
        let graph = gb.finalize(&st);
        let exp = "digraph{
 rankdir=\"LR\";
 { edge [style=invis weight=100];n0->n2->n4->n7}
n0 [label=\"na []\"];
n2 [label=\"na []\"];
n4 [label=\"nb []\"];
n7 [label=\"nc []\"];
n0 -> n4 [label=\"a\"];
n2 -> n7 [label=\"a\"];
n4 -> n7 [label=\"b\"];
}";
        assert_eq!(graph.to_dot(&gb.spec), exp);
    }

    #[test]
    fn test_data_generation() {
        let mut gs = GraphSpec::new();
        let d_u8 = Some(gs.data_type("u8", Arc::new(atomic_data::DataInt::new(1, vec!()))));
        let _n_a = gs.node_type("u8_1", d_u8, vec![], vec![], vec![]);
        let _n_b = gs.node_type("u8_2", d_u8, vec![], vec![], vec![]);
        let dist = crate::random::distributions::Distributions::new();
        let mut gb = GraphBuilder::new(Rc::new(gs));
        let mutator = primitive_mutator::mutator::PrimitiveMutator::new();
        let mut st = VecGraph::empty();
        gb.start(&mut st);
        gb.append_random(10, &mutator, &mut st, &dist).unwrap();
        assert_eq!(gb.finalize(&st).data_len(), 10);
    }


    #[test]
    fn test_fuzz_splice_random_limited_data() {
        use std::fs::File;

        use crate::graph_mutator::graph_storage::RefGraph;
        use crate::graph_mutator::spec_loader;
        use crate::mutator::Mutator;

        let file = File::open("interpreter/build/spec.msgp").unwrap();
        let gs = spec_loader::load_spec_from_read(file);
        let ops = Box::leak(Box::new([0u16; 512]));
        let data = Box::leak(Box::new([0u8; 512]));
        let dist = crate::random::distributions::Distributions::new();
        let mut storage = RefGraph::new(ops, data, Box::leak(Box::new(0)), Box::leak(Box::new(0)));
        let mut mutator = Mutator::new(gs);
        mutator.generate(100, &mut storage, &dist);
        let graph = mutator.dump_graph(&storage);
        let cnt = 1000;
        for _i in 0..cnt {
            mutator.splice_random(&graph, &custom_dict::CustomDict::new(), &mut storage, &dist);
        }
    }
}
