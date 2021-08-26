mkdir ../sharedir
mkdir ../sharedir_asan

bash compile.sh
cp bin/req_data ../sharedir/req_data
cp bin/hypertrash_crash_detector ../sharedir/hypertrash_crash_detector
cp bin/hypertrash_crash_detector_asan ../sharedir/hypertrash_crash_detector_asan
cp bin/set_ip_range ../sharedir/set_ip_range
cp bin/set_kvm_ip_range ../sharedir/set_kvm_ip_range
cp src/run.sh ../sharedir/run.sh
cp bin/req_data ../sharedir/req_data

cp bin/req_data ../sharedir_asan/req_data
cp bin/hypertrash_crash_detector ../sharedir_asan/hypertrash_crash_detector
cp bin/hypertrash_crash_detector_asan ../sharedir_asan/hypertrash_crash_detector_asan
cp bin/set_ip_range ../sharedir_asan/set_ip_range
cp bin/set_kvm_ip_range ../sharedir_asan/set_kvm_ip_range
cp src/run_asan.sh ../sharedir_asan/run.sh
cp bin/req_data ../sharedir_asan/req_data

