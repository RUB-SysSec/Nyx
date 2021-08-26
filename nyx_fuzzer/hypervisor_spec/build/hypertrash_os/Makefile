#
# HyperCube OS
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# 


AS=as --32
CC=gcc-8 -m32

CFLAGS=-c -std=gnu99 -ffreestanding -O1 -Wall -Wextra -static -DHYPERTRASH -DLOOP
LDFLAGS=-T misc/linker.ld -ffreestanding -O1 -nostdlib -static
LIBS=
INCLUDE=-Iinclude -Itesseract

AS_SOURCES= src/asm/boot.s src/asm/cpu_tables.s src/asm/ap_boot.s
AS_OBJECTS=$(AS_SOURCES:.s=.o)

SOURCES=src/kernel.c src/libk.c src/serial.c src/tty.c src/gdt.c src/idt.c src/isr.c src/pci.c src/panic.c src/msr.c src/acpi.c src/cpuid.c src/apic.c src/pic.c src/mboot.c src/efi.c src/mem.c src/smp.c src/fuzz.c src/isa.c tesseract/core.c tesseract/state.c tesseract/opcodes.c tesseract/decompiler.c tesseract/dict.c src/hypercube_opcodes.c
OBJECTS=$(SOURCES:.c=.o) bin/logo.o

.PHONY: all clean run 

all: clean misc/linker.ld $(AS_SOURCES) $(SOURCES) hypertrash_os.bin 

bin/logo.o:
	objcopy -I binary -O elf32-i386 -B i386 misc/logo.bmp bin/logo.o

bin/payload.o:
	objcopy -I binary -O elf32-i386 -B i386 misc/crash.hexa bin/payload.o

hypertrash_os_crash.bin: CFLAGS := $(CFLAGS) -DPAYLOAD
hypertrash_os_crash.bin:  $(AS_OBJECTS) $(OBJECTS) bin/payload.o
	$(CC) $(LDFLAGS) -o bin/hypertrash_os.bin $(AS_OBJECTS) $(OBJECTS) bin/payload.o $(LIBS)
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypertrash_os.bin isofiles/boot/kernel.bin
	./grub/bios/bin/grub-mkrescue --xorriso=./xorriso -d ./grub/bios/lib/grub/i386-pc/ -o iso/hypertrash_os_bios_crash.iso  --install-modules='normal multiboot2' --locales= --themes=None --modules= --fonts= isofiles 
	rm -r isofiles/

hypertrash_os.bin: $(AS_OBJECTS) $(OBJECTS)
	$(CC) $(LDFLAGS) -o bin/hypertrash_os.bin $(AS_OBJECTS) $(OBJECTS) $(LIBS)
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypertrash_os.bin isofiles/boot/kernel.bin
	./grub/bios/bin/grub-mkrescue --xorriso=./xorriso -d ./grub/bios/lib/grub/i386-pc/ -o iso/hypertrash_os_bios.iso  --install-modules='normal multiboot2' --locales= --themes=None --modules= --fonts=  isofiles 
	rm -r isofiles/

.s.o:
	$(AS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE)

clean:
	rm -rf isofiles/
	rm -f $(AS_OBJECTS)
	rm -f $(OBJECTS)
	rm -f bin/hypertrash_os.bin
	rm -f iso/hypertrash_os_bios.iso
	rm -f iso/hypertrash_os_efi.iso
	rm -f iso/hypertrash_os_efi_app.iso
	rm -f serial.txt
	rm -f bin/payload.o
