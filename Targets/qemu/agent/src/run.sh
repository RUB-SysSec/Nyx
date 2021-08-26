chmod +x req_data
du -hs req_data
sha1sum req_data

./req_data hypertrash.iso hypertrash.iso
./req_data hypertrash_crash_detector hypertrash_crash_detector
./req_data hypertrash_crash_detector_asan hypertrash_crash_detector_asan
./req_data set_kvm_ip_range set_kvm_ip_range
./req_data set_ip_range set_ip_range

chmod +x hypertrash_crash_detector
chmod +x hypertrash_crash_detector_asan
chmod +x set_kvm_ip_range
chmod +x set_ip_range

#./set_kvm_ip_range
#./set_ip_range 0x1000 0x7ffffffff000 1

# disable ASLR
echo 0 > /proc/sys/kernel/randomize_va_space

./set_ip_range 0x1000 0x7ffffffff000 0

echo 0 > /proc/sys/kernel/printk

clear

LD_PRELOAD=./hypertrash_crash_detector qemu-system-x86_64 -nographic -cdrom hypertrash.iso -m 100 -enable-kvm -device virtio-gpu-pci -device nec-usb-xhci -nographic

