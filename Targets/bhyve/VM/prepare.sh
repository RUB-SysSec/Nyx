wget https://download.freebsd.org/ftp/releases/ISO-IMAGES/11.3/FreeBSD-11.3-RELEASE-amd64-disc1.iso

qemu-img create -f qcow2 bhyve.qcow2 20G

sudo modprobe kvm
sudo modprobe kvm-intel

sudo chmod 777 /dev/kvm

qemu-system-x86_64 --enable-kvm -m 1024 -cdrom FreeBSD-11.3-RELEASE-amd64-disc1.iso -k de -hda bhyve.qcow2 -nic user,hostfwd=tcp::2222-:22 -vnc :0

