chmod +x req_data
du -hs req_data

./req_data hypertrash.iso hypertrash.iso
./req_data hypertrash_crash_detector hypertrash_crash_detector
./req_data hypertrash_crash_detector_asan hypertrash_crash_detector_asan
./req_data set_vmm_ip_range set_vmm_ip_range
./req_data set_ip_range set_ip_range

chmod +x hypertrash_crash_detector
chmod +x hypertrash_crash_detector_asan
chmod +x set_vnmm_ip_range
chmod +x set_ip_range

#./set_vmm_ip_range
#./set_ip_range 0x1000 0x7ffffffff000 1

# disable ASLR
# ASLR is disabled by default ? 

./set_ip_range 0x1000 0x7ffffffff000 0

clear


bhyvectl --vm=testvm --destroy; 
env LD_PRELOAD=./hypertrash_crash_detector_asan ASAN_OPTIONS=verbosity=1:log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false  /usr/src/usr.sbin/bhyve/bhyve.full  -w -H -s 0:0,hostbridge -s 1:0,lpc -s 2:0,ahci-cd,hypertrash.iso -l com1,stdio -s 29,fbuf,tcp=0.0.0.0:5900,w=800,h=600 -s 30,xhci,tablet -c 1 -m 320M -l bootrom,/usr/local/share/uefi-firmware/BHYVE_UEFI_CSM.fd testvm
