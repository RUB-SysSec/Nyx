cd /home/user
export ASSUME_ALWAYS_YES=yes
yes | pkg install bhyve-firmware 
echo "vm.pmap.pti=0" > /boot/loader.conf
clang -I ./ loader.c -o loader
clang -I ./ req_data.c -o req_data
shutdown -h now
