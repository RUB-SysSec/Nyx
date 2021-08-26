mkdir bin/ 2> /dev/null

printf "\tCompiling QEMU/KVM loader...\n"
gcc src/loader.c -I ./ -o bin/loader

printf "\tCompiling req_data tool...\n"
gcc src/req_data.c -I ./ -o bin/req_data

printf "\tCompiling set_ip_range tool...\n"
gcc src/set_ip_range.c -I ./ -o bin/set_ip_range

printf "\tCompiling set_kvm_ip_range tool...\n"
gcc src/set_kvm_ip_range.c -I ./ -o bin/set_kvm_ip_range

printf "\tCompiling hypertrash_crash_detector...\n"
gcc -I./ -shared -O0 -m64 -Werror -fPIC src/hypertrash_crash_detector.c -o bin/hypertrash_crash_detector -ldl

printf "\tCompiling hypertrash_crash_detector_asan...\n"
gcc -I./ -shared -O0 -m64 -Werror -fPIC src/hypertrash_crash_detector_asan.c -o bin/hypertrash_crash_detector_asan -ldl
