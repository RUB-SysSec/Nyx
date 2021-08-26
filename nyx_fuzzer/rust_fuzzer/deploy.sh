cargo build --release
ssh kafl_02  'pkill -9 qemu' 
ssh kafl_02  'pkill -9 fuzz' 
scp target/release/rust_fuzzer kafl_02:/tmp/fuzz
