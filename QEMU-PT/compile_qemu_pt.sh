unzip qemu-4.2.50.zip
patch -p0 < qemu_pt.patch
cd qemu
sh compile.sh
cd -

