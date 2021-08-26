//extern crate structured_fuzzer;
extern crate rand;

use std::fs::File;

use structured_fuzzer::graph_mutator::graph_storage::{VecGraph};
use structured_fuzzer::graph_mutator::spec_loader;
use structured_fuzzer::GraphStorage;
use clap::{App, Arg};

fn main() {

    let matches = App::new("generator")
    .about("Generate strings using a grammar. This can also be used to generate a corpus")
    .arg(Arg::with_name("spec_path")
         .short("s")
         .value_name("SPEC")
         .takes_value(true)
         .required(true)
         .help("Path to the spec.msgp"))
     .arg(Arg::with_name("input_path")
         .short("i")
         .value_name("IN_PATH")
         .takes_value(true)
         .help("Which .bin file to read"))
    .arg(Arg::with_name("output_path")
         .short("o")
         .value_name("OUT_PATH")
         .takes_value(true)
         .help("Where to store outputs"))
    .arg(Arg::with_name("svg")
         .value_name("SVG")
         .help("dump output as svg"))
    .get_matches();

    let spec_path = matches
    .value_of("spec_path")
    .expect("spec_path is a required parameter")
    .to_string();
    let input_path = matches.value_of("input_path").expect("input path is a reuqired parameter").to_string();
    let output_path = matches.value_of("output_path").expect("output path is a reuqired parameter").to_string();
    let svg = matches.is_present("svg");

    let file = File::open(spec_path).unwrap();
    let gs = spec_loader::load_spec_from_read(file);

    let graph = VecGraph::new_from_bin_file(&input_path, &gs);
    if svg {
        graph.to_svg(&output_path, &gs);
    } else {
        println!("{}", graph.to_script(&gs));
        graph.write_to_script_file(&output_path, &gs);
    }
}
