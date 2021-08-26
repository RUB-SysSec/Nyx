path=$PWD
cp Makefile.template Makefile
sed -i "s|-IPWD|-I"$path"|g" Makefile
make -C /lib/modules/`uname -r`/build M=$PWD
sudo rmmod kvm-pt 2> /dev/null
sudo insmod kvm-pt.ko #pml=n
sudo rmmod kvm-pt 
sudo insmod kvm-pt.ko #pml=n
sudo chmod 777 /dev/kvm-pt
