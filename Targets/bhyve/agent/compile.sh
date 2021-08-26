mkdir bin/ 2> /dev/null

printf "\tCompiling Bhyve loader...\n"
clang src/loader.c -I ../ -o bin/loader

printf "\tCompiling req_data tool...\n"
clang src/req_data.c -I ../ -o bin/req_data

printf "\tCompiling set_ip_range tool...\n"
clang src/set_ip_range.c -I ../ -o bin/set_ip_range

printf "\tCompiling set_kvm_ip_range tool...\n"
clang src/set_vmm_ip_range.c -I ../ -o bin/set_vmm_ip_range
