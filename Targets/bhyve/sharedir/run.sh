#!/bin/sh
chmod +x req_data
du -hs req_data

kldload vmm

./req_data kafl_user.h kafl_user.h
./req_data hypertrash.iso hypertrash.iso
./req_data hypertrash_crash_detector.c hypertrash_crash_detector.c
./req_data set_vmm_ip_range set_vmm_ip_range
./req_data set_ip_range.c set_ip_range.c

clang  -I ./ -shared -O0 -m64 -Werror -fPIC ./hypertrash_crash_detector.c -o ./hypertrash_crash_detector -ldl
clang  -I ./ -O0 -m64 set_ip_range.c -o set_ip_range

#./set_vmm_ip_range
#./set_ip_range 0x1000 0x7ffffffff000 1
./set_ip_range 0x1000 0x100000000 0


ifconfig tap0 create
sysctl net.link.tap.up_on_open=1
ifconfig bridge0 create
ifconfig bridge0 addm lo addm tap0
ifconfig bridge0 up

dd if=/dev/zero of=disk.img bs=1024 count=64

bhyvectl --vm=testvm --destroy; 

env MALLOC_PERTURB_=255 MALLOC_MMAP_PERTURB_=255 MALLOC_CHECK_=3 LD_PRELOAD=./hypertrash_crash_detector  /usr/sbin/bhyve  -w -H -s 0:0,hostbridge -s 1:0,lpc -s 2:0,ahci-cd,hypertrash.iso -l com1,stdio -s 3,fbuf,tcp=0.0.0.0:5900,w=800,h=600 -s 4:0,ahci-hd,hd:disk.img -c 1 -m 320M -l bootrom,/usr/local/share/uefi-firmware/BHYVE_UEFI_CSM.fd testvm 

