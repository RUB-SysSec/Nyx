GRUB_VERSION="2.02"
GRUB_URL="ftp://ftp.gnu.org//gnu/grub/grub-2.02.tar.gz"
GRUB_MD5="1116d1f60c840e6dbd67abbc99acb45d"
PWD_BIOS=$(pwd)/bios/
PWD_EFI=$(pwd)/efi/
PWD_EFI_APP=$(pwd)/efi_app/

echo "[*] Installing build dependencies for GRUB2 $GRUB_VERSION ..."
sudo -Eu root apt-get build-dep grub2 -y > /dev/null

echo "[*] Downloading GRUB2 $GRUB_VERSION ..."
wget -O grub.tar.gz $GRUB_URL 2> /dev/null

echo "[*] Checking signature of GRUB2 $GRUB_VERSION ..."
CHKSUM=`md5sum grub.tar.gz| cut -d' ' -f1`

if [ "$CHKSUM" != "$GRUB_MD5" ]; then
  echo "[-] Error: signature mismatch..."
  exit 1
fi

echo "[*] Unpacking GRUB2 $GRUB_VERSION ..."
tar xf grub.tar.gz

echo "[*] Compiling GRUB2 $GRUB_VERSION BIOS bootloader ..."
cd grub-2.02/
CFLAGS=-Wno-error=unused-value ./autogen.sh
CFLAGS=-Wno-error=unused-value ./configure --target=i386 --with-platform="pc" --prefix=$PWD_BIOS 
CFLAGS=-Wno-error=unused-value make -j 8 
make install 
make clean
cd .. 

rm -rf ./grub-2.02/
rm -f grub.tar.gz
echo "[*] Done ..."
