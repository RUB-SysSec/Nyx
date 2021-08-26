
cd ../agent
sh compile.sh
cd -
scp -P 2223 ../agent/bin/loader ../misc/grub.cfg  ../misc/install.sh  ../misc/sources.list  user@localhost:/home/user/


