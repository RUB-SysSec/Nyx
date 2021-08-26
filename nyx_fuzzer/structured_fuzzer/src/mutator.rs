use std::rc::Rc;
use std::borrow::Borrow;
use std::sync::Arc;

use crate::graph_mutator::graph_builder::GraphBuilder;
use crate::graph_mutator::graph_iter::GraphNode;
use crate::graph_mutator::graph_storage::GraphStorage;
use crate::graph_mutator::graph_storage::VecGraph;
use crate::graph_mutator::spec::GraphSpec;
use crate::primitive_mutator::mutator::PrimitiveMutator;
use crate::random::distributions::Distributions;
use crate::custom_dict::CustomDict;

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
pub enum MutationStrategy{
    GenerateTail(GenerateTail),
    SpliceRandom,
    Splice,
    DataOnly,
    Generate,
    Repeat,
    Minimize,
    MinimizeSplit,
}

impl MutationStrategy{
    pub fn name(&self) -> &str{
        match self{
            MutationStrategy::GenerateTail(_) => "generate_tail",
            MutationStrategy::SpliceRandom => "splice_random",
            MutationStrategy::Splice => "splice",
            MutationStrategy::DataOnly => "data_only",
            MutationStrategy::Generate => "generate",
            MutationStrategy::Repeat => "repeat",
            MutationStrategy::Minimize => "minimize",
            MutationStrategy::MinimizeSplit => "minimize_split",
        }
    }
}

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
pub struct GenerateTail{ pub drop_last: usize, pub generate: usize }

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
pub enum NodeMutationType {
    CopyNode,
    MutateNodeData,
    DropNode,
    SkipAndGenerate,
}

pub struct Mutator {
    pub spec: Rc<GraphSpec>,
    builder: GraphBuilder,
    mutator: PrimitiveMutator,
}

pub trait InputQueue {
    fn sample_for_splicing(&self, dist: &Distributions) -> Arc<VecGraph>;
}

impl InputQueue for Vec<VecGraph>{
    fn sample_for_splicing(&self, dist: &Distributions) -> Arc<VecGraph>{
        assert!(self.len() > 0);
        return Arc::new(self[dist.gen_range(0,self.len())].clone());
    }
}

impl Mutator {
    pub fn new(spec: GraphSpec) -> Self {
        let spec = Rc::new(spec);
        let mutator = PrimitiveMutator::new();
        let builder = GraphBuilder::new(spec.clone());

        return Self {
            spec,
            builder,
            mutator,
        };
    }

    pub fn mutate<S: GraphStorage, Q: InputQueue>(&mut self, orig: &VecGraph, dict: &CustomDict, queue: &Q, storage: &mut S, dist: &Distributions) -> MutationStrategy{
        if orig.op_len() == 0 {
            self.generate(50, storage, dist);
            return MutationStrategy::Generate;
        }
        let orig_len =  orig.node_len(&self.spec);
        let strategy = dist.gen_mutation_strategy(orig_len);
        match strategy{
            MutationStrategy::GenerateTail(args) => self.generate_tail(orig, args, storage, dist),
            MutationStrategy::SpliceRandom => self.splice_random(orig, dict, storage, dist),
            MutationStrategy::Splice => self.splice(orig, queue, storage, dist),
            MutationStrategy::DataOnly => self.mutate_data(orig, dict, storage, dist),
            MutationStrategy::Generate => self.generate(50, storage, dist),
            MutationStrategy::Repeat => self.repeat(orig, dict, storage, dist),
            MutationStrategy::Minimize => unreachable!(),
            MutationStrategy::MinimizeSplit => unreachable!(),
        }
        return strategy;
    }

    pub fn repeat<S: GraphStorage>(&mut self, orig: &VecGraph, dict: &CustomDict, storage: &mut S, dist: &Distributions) {
        self.builder.start(storage);

        let repeat_nodes = dist.gen_range(0, 4);
        let insert_pos = dist.gen_range(0, orig.node_len(&self.spec));


        for n in orig.node_iter(&self.spec.clone()).take(insert_pos) {
            if self.builder.is_full(storage){return;}
            self.builder.append_node(&n, storage, dist); 
        }

        let origs = orig.node_iter(&self.spec).skip(insert_pos).take(repeat_nodes).collect::<Vec<_>>();
        for _ in 0..repeat_nodes {
            for n in origs.iter() {
                if self.builder.is_full(storage){return;}
                self.builder.append_node_mutated(&n, dict, &self.mutator, storage, dist);
            }
        }

        for n in orig.node_iter(&self.spec.clone()).skip(insert_pos) {
            if self.builder.is_full(storage){return;}
            self.builder.append_node(&n, storage, dist); 
        }
    }

    pub fn generate_tail<S: GraphStorage>(&mut self, orig: &VecGraph, args: GenerateTail, storage: &mut S, dist: &Distributions){
        let orig_len =  orig.node_len(&self.spec);

        self.builder.start(storage);
        for n in orig.node_iter(&self.spec).take(orig_len-args.drop_last) {
            if self.builder.is_full(storage){return;}
            self.builder.append_node(&n, storage, dist);
        }
        
        self.builder.append_random(args.generate, &self.mutator, storage, dist).unwrap();
    }

    pub fn mutate_data<S: GraphStorage>(&mut self, orig: &VecGraph, dict: &CustomDict,  storage: &mut S, dist: &Distributions) {
        self.builder.start(storage);
        for n in orig.node_iter(&self.spec.clone()) {
            if self.builder.is_full(storage){return;}
            if dist.should_mutate_data(){
                self.builder.append_node_mutated(&n, dict, &self.mutator,  storage, dist);
            } else {
                self.builder.append_node(&n, storage, dist); 
            }
        }
    }


    pub fn splice_random<S: GraphStorage>(&mut self, orig: &VecGraph, dict: &CustomDict, storage: &mut S, dist: &Distributions) {
        self.builder.start(storage);
        for n in orig.node_iter(&self.spec.clone()) {
            if self.builder.is_full(storage){return;}
            let mutation = self.pick_op(&n, dist);
            self.apply_graph_node(mutation, &n, dict, storage, dist);
        }
    }

    pub fn pick_splice_points(&self, len: usize, dist: &Distributions) -> Vec<usize>{
        use std::cmp::Reverse;
        let num = match len{
            0 => unreachable!(),
            1..=3 => dist.gen_range(1,3),
            4..=15 => dist.gen_range(1,5),
            _ => dist.gen_range(4,16),
        };
        let mut res = (0..num).map(|_| dist.gen_range(0,len) ).collect::<Vec<_>>();
        res.sort_unstable();
        res.sort_by_key(|x| (*x, Reverse(*x))); 
        res.dedup();
        return res;
    }

    pub fn splice< S: GraphStorage, Q:InputQueue >(&mut self, orig: &VecGraph, queue: &Q, storage: &mut S, dist: &Distributions) {

        let orig_len = orig.node_len(&self.spec);
        let mut splice_points = self.pick_splice_points(orig_len, dist);

        self.builder.start(storage);
        
        for (i,n) in orig.node_iter(&self.spec.clone()).enumerate() {
            if self.builder.is_full(storage){return;}
            if splice_points.len() > 0 && i == *splice_points.last().unwrap(){
                splice_points.pop();
                let other_lock = queue.sample_for_splicing(&dist);
                let other = other_lock.as_ref();
                let other_len = other.node_len(&self.spec);
                let start = dist.gen_range(0,other_len);
                let mut len = dist.gen_range(1,16);
                if len > other_len-start { len = other_len-start }
                for nn in other.node_iter(&self.spec.clone()).skip(start).take(len){
                    self.builder.append_node(&nn, storage, dist);
                }
            }
            self.builder.append_node(&n, storage, dist);
        }
    }

    pub fn generate<S: GraphStorage>(&mut self, n: usize, storage: &mut S, dist: &Distributions) {
        self.builder.start(storage);
        self.builder
            .append_random(n, &self.mutator, storage, dist)
            .unwrap();
    }

    pub fn drop_range<S: GraphStorage>(
        &mut self,
        orig: &VecGraph,
        range: std::ops::Range<usize>,
        storage: &mut S,
        dist: &Distributions
    ) {
        self.builder.start(storage);
        for (i, n) in orig.node_iter(&self.spec.clone()).enumerate() {
            if range.start <= i && i < range.end {
                continue;
            }
            self.builder.append_node(&n, storage, dist);
        }
    }

    pub fn drop_node_at<S: GraphStorage>(&mut self, orig: &VecGraph, i: usize, storage: &mut S, dist: &Distributions) {
        self.drop_range(orig, i..i + 1, storage, dist);
    }

    pub fn dump_graph<S: GraphStorage>(&self, storage: &S) -> VecGraph {
        storage.as_vec_graph()
    }

    fn apply_graph_node<S: GraphStorage>(
        &mut self,
        op: NodeMutationType,
        n: &GraphNode,
        dict: &CustomDict,
        storage: &mut S,
        dist: &Distributions
    ) {
        use NodeMutationType::*;

        match op {
            CopyNode => {
                self.builder.append_node(&n, storage, dist);
            }
            MutateNodeData => self.builder.append_node_mutated(&n, dict, &self.mutator, storage, dist),
            DropNode => {}
            SkipAndGenerate => {
                let len = dist.gen_number_of_random_nodes();
                self.builder
                    .append_random(len, &self.mutator, storage, dist)
                    .unwrap();
            }
        }
    }

    fn pick_op(&self, _n: &GraphNode, dist: &Distributions) -> NodeMutationType {
        return dist.gen_graph_mutation_type();
    }

    pub fn num_ops_used<S: GraphStorage>(&self, storage: &S) -> usize {
        return self.builder.num_ops_used(storage);
    }
}
