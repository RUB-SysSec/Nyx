//extern crate structured_fuzzer;
extern crate rand;

use std::fs::File;
use std::io::prelude::*;

use structured_fuzzer::graph_mutator::graph_storage::{VecGraph};
use structured_fuzzer::graph_mutator::spec_loader;
use structured_fuzzer::mutator::Mutator;
use structured_fuzzer::GraphStorage;
use structured_fuzzer::random::distributions::Distributions;
use structured_fuzzer::custom_dict::CustomDict;

use clap::{App, Arg, value_t};

fn main() {

    let matches = App::new("generator")
    .about("Generate strings using a grammar. This can also be used to generate a corpus")
    .arg(Arg::with_name("spec_path")
         .short("s")
         .value_name("SPEC")
         .takes_value(true)
         .required(true)
         .help("Path to the spec.msgp"))
    .arg(Arg::with_name("length")
         .short("n")
         .value_name("LENGTH")
         .takes_value(true)
         .help("Length of the generated scripts [default: 10]"))
    .arg(Arg::with_name("number_of_mutations")
         .short("m")
         .value_name("NUMBER")
         .takes_value(true)
         .help("Number of mutations to generate [default: 0]"))
    .arg(Arg::with_name("output_path")
         .short("o")
         .value_name("OUT_PATH")
         .takes_value(true)
         .help("Where to store outputs"))
    .get_matches();

    let spec_path = matches
    .value_of("spec_path")
    .expect("spec_path is a required parameter")
    .to_string();
    let length = value_t!(matches, "length", usize).unwrap_or(10);
    let number_of_mutations = value_t!(matches, "number_of_mutations", usize).unwrap_or(0);
    let out_path = matches.value_of("output_path").unwrap_or("./").to_string();


    let file = File::open(spec_path).unwrap();
    let gs = spec_loader::load_spec_from_read(file);
    //let ops = Box::leak(Box::new([0u16; 512]));
    //let data = Box::leak(Box::new([0u8; 512]));
    //let mut storage = RefGraph::new(ops, data, Box::leak(Box::new(0)), Box::leak(Box::new(0)));
    let mut storage = VecGraph::empty();
    let mut mutator = Mutator::new(gs);
    let dist = Distributions::new();
    mutator.generate(length, &mut storage, &dist);
    let graph = mutator.dump_graph(&storage);
    let mut file = File::create(&format!("{}/out.dot",out_path)).unwrap();
    graph.to_svg(&format!("{}/out.svg",out_path), &mutator.spec);
    file.write_all(&graph.to_dot(&mutator.spec).as_bytes()).unwrap();

    let queue = vec!(graph);
    let graph = &queue[0];

    let cnt = number_of_mutations;
    for i in 0..cnt {
        if cnt > 100 && i % (cnt / 100) == 0 {
            println!("mutating {}%...", i / (cnt / 100));
        }
        let strategy  = mutator.mutate(graph, &CustomDict::new(), &queue, &mut storage, &dist);
        let g2 = mutator.dump_graph(&storage);
        let mut file = File::create(format!("{}/out_mut_{}_{}.dot", out_path, strategy.name(), i)).unwrap();
        file.write_all(&g2.to_dot(&mutator.spec).as_bytes()).unwrap();
        g2.to_svg(&format!("{}/out_mut_{}_{}.svg",out_path,strategy.name(), i), &mutator.spec);
    }
}
