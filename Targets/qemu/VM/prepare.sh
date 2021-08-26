wget http://old-releases.ubuntu.com/releases/18.04.4/ubuntu-18.04.4-live-server-amd64.iso

qemu-img create -f qcow2 qemu.qcow2 20G

sudo modprobe kvm
sudo modprobe kvm-intel

sudo chmod 777 /dev/kvm

qemu-system-x86_64 --enable-kvm -m 1024 -cdrom ubuntu-18.04.4-live-server-amd64.iso -k de -hda qemu.qcow2  -nic user,hostfwd=tcp::2223-:22 -vnc :0

