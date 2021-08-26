import sys
sys.path.insert(1, '../../structured_fuzzer/interpreter/')

from spec_lib.graph_spec import *
from spec_lib.data_spec import *
from spec_lib.graph_builder import *
from spec_lib.generators import opts,flags,limits

import jinja2

d_u32 = None
d_u16 = None
d_u8 = None

def make_globals(s):
  global d_u8, d_u16, d_u32
  d_u32 = s.data_u32("u32")
  d_u16 = s.data_u16("u16")
  d_u8 = s.data_u8("u8")

def autogen_write_32(node, arg):
    var = "data_write_mmio_%x_32"%arg
    return "*(volatile uint32_t*)(%s->addr)=%s->data;\n"%(var,var)

def autogen_read_u32(node, arg):
    return "volatile uint32_t tmp = *(volatile uint32_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u32(node, arg):
    return "*(volatile uint32_t*)(0x%x)=*data_u32;\n"%arg

def autogen_read_u16(node, arg):
    return "volatile uint16_t tmp = *(volatile uint16_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u16(node, arg):
    return "*(volatile uint16_t*)(0x%x)=*data_u16;\n"%arg

def autogen_read_u8(node, arg):
    return "volatile uint8_t tmp = *(volatile uint8_t*)(0x%x);\n(void)(tmp);\n"%arg

def autogen_write_u8(node, arg):
    return "*(volatile uint8_t*)(0x%x)=*data_u8;\n"%arg


def autogen_pass(node, arg):
    return arg


def make_hypercube_generic(s):
    #s.includes.append("\"./hypertrash_os/src/hypercube_opcodes.h\"")

    
    s.set_node_code_gen(autogen_pass)

    d_set_scratch_area = s.data_struct("d_set_scratch_area")
    d_set_scratch_area.u16("offset")
    d_set_scratch_area.u16("len")
    d_set_scratch_area.u32("seed")
    d_set_scratch_area.finalize()

    set_scratch_area = "hc_set_scratch_area(0, data_d_set_scratch_area->offset, data_d_set_scratch_area->seed, data_d_set_scratch_area->len);\n"
    s.node_type("set_scratch_area", data=d_set_scratch_area, codegen_args=set_scratch_area);

    i = 0
    d_set_scratch_area = s.data_struct("d_set_scratch_area_2_%d"%(i))
    d_set_scratch_area.u16("len")
    d_set_scratch_area.u32("seed")
    d_set_scratch_area.finalize()

    set_scratch_area = "hc_set_scratch_area_2(0, data_d_set_scratch_area_2_%d->seed, data_d_set_scratch_area_2_%d->len);\n"%(i, i)
    s.node_type("d_set_scratch_area_2_%d"%(i), data=d_set_scratch_area, codegen_args=set_scratch_area);




def make_hypercube(s, base, size, region_id, mmio=True):

    s.set_node_code_gen(autogen_pass)
 
    
    if mmio:
        set_scratch_area_ptr = "hc_mmio_write_scratch_ptr(0x%x, 0x%x, *data_u32);\n"%(base, size)
    else:
        set_scratch_area_ptr = "hc_io_write_scratch_ptr(0x%x, 0x%x, *data_u32);\n"%(base, size)

    s.node_type("set_scratch_area_ptr_%d"%(region_id), data=d_u32, codegen_args=set_scratch_area_ptr);
    


    for i in (8, 16, 32):
        d_write = s.data_struct("d_write_%d_%d"%(i, region_id))
        d_write.u32("offset")
        if i == 8:
            d_write.u32("data")
        if i == 16:
            d_write.u16("data")
        if i == 32:
            d_write.u8("data")
        d_write.finalize()

    if mmio:
        mmio_write = "hc_mmio_write_%d(0x%x, 0x%x, data_d_write_%d_%d->offset, data_d_write_%d_%d->data);\n"%(i, base, size, i, region_id, i, region_id)
    else:
        mmio_write = "hc_io_write_%d(0x%x, 0x%x, data_d_write_%d_%d->offset, data_d_write_%d_%d->data);\n"%(i, base, size, i, region_id, i, region_id)

    s.node_type("mmio_write_%d_%d"%(i, region_id), data=d_write, codegen_args=mmio_write);

    for i in (8, 16, 32):
        if mmio:
            mmio_read = "hc_mmio_read_%d(0x%x, 0x%x, *data_u32);\n"%(i, base, size)
        else:
            mmio_read = "hc_io_read_%d(0x%x, 0x%x, *data_u32);\n"%(i, base, size)
        s.node_type("mmio_read_%d_%d"%(i, region_id), data=d_u32, codegen_args=mmio_read);

    for i in (8, 16, 32):
        d_write = s.data_struct("d_xor_%d_%d"%(i, region_id))
        d_write.u32("offset")
        if i == 8:
            d_write.u32("mask")
        if i == 16:
            d_write.u16("mask")
        if i == 32:
            d_write.u8("mask")
        d_write.finalize()

        if mmio:
            xor_write = "hc_mmio_xor_%d(0x%x, 0x%x, data_d_xor_%d_%d->offset, data_d_xor_%d_%d->mask);\n"%(i, base, size, i, region_id, i, region_id)
        else:
            xor_write = "hc_io_xor_%d(0x%x, 0x%x, data_d_xor_%d_%d->offset, data_d_xor_%d_%d->mask);\n"%(i, base, size, i, region_id, i, region_id)

        s.node_type("xor_write_%d_%d"%(i, region_id), data=d_write, codegen_args=xor_write);


    for i in (8, 16, 32):
        d_bruteforce = s.data_struct("d_bruteforce_%d_%d"%(i, region_id))
        d_bruteforce.u32("offset")
        if i == 8:
            d_bruteforce.u32("data")
        if i == 16:
            d_bruteforce.u16("data")
        if i == 32:
            d_bruteforce.u8("data")
        d_bruteforce.u8("num")
        d_bruteforce.finalize()

        if mmio:
            bruteforce_write = "hc_mmio_write_bruteforce_%d(0x%x, 0x%x, data_d_bruteforce_%d_%d->offset, data_d_bruteforce_%d_%d->data, data_d_bruteforce_%d_%d->num);\n"%(i, base, size, i, region_id, i, region_id, i, region_id)
        else:
            bruteforce_write = "hc_io_write_bruteforce_%d(0x%x, 0x%x, data_d_bruteforce_%d_%d->offset, data_d_bruteforce_%d_%d->data, data_d_bruteforce_%d_%d->num);\n"%(i, base, size, i, region_id, i, region_id, i, region_id)

        s.node_type("write_bruteforce_%d_%d"%(i, region_id), data=d_bruteforce, codegen_args=bruteforce_write);

    for i in (8, 16, 32):
        d_memset = s.data_struct("d_memset_%d_%d"%(i, region_id))
        d_memset.u32("offset")
        if i == 8:
            d_memset.u32("data")
        if i == 16:
            d_memset.u16("data")
        if i == 32:
            d_memset.u8("data")
        d_memset.u16("num")
        d_memset.finalize()

        if mmio:
            memset = "hc_mmio_memset_%d(0x%x, 0x%x, data_d_memset_%d_%d->offset, data_d_memset_%d_%d->data, data_d_memset_%d_%d->num);\n"%(i, base, size, i, region_id, i, region_id, i, region_id)
        else:
            memset = "hc_io_memset_%d(0x%x, 0x%x, data_d_memset_%d_%d->offset, data_d_memset_%d_%d->data, data_d_memset_%d_%d->num);\n"%(i, base, size, i, region_id, i, region_id, i, region_id)

        s.node_type("memset_%d_%d"%(i, region_id), data=d_memset, codegen_args=memset);
    
    if not mmio:

        for i in (8, 16, 32):
            d_memset = s.data_struct("d_writes_%d_%d"%(i, region_id))
            d_memset.u32("offset")
            d_memset.u16("data")
            d_memset.u16("num")
            d_memset.finalize()

            memset = "hc_io_writes_%d(0x%x, 0x%x, data_d_writes_%d_%d->offset, data_d_writes_%d_%d->data, data_d_writes_%d_%d->num);\n"%(i, base, size, i, region_id, i, region_id, i, region_id)

            s.node_type("writes_%d_%d"%(i, region_id), data=d_memset, codegen_args=memset);
        
        for i in (8, 16, 32):
            d_memset = s.data_struct("d_reads_%d_%d"%(i, region_id))
            d_memset.u32("offset")
            d_memset.u16("num")
            d_memset.finalize()

            memset = "hc_io_reads_%d(0x%x, 0x%x, data_d_reads_%d_%d->offset, data_d_reads_%d_%d->num);\n"%(i, base, size, i,  region_id, i, region_id)

            s.node_type("reads_%d_%d"%(i, region_id), data=d_memset, codegen_args=memset);
            
"""
brctl addbr br0
ip addr flush dev eth0
brctl addif br0 eth0
tunctl -t tap0 -u `whoami`
brctl addif br0 tap0
ifconfig eth0 up
ifconfig tap0 up
ifconfig br0 up
"""

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device pcnet,netdev=net0 
-netdev tap,id=net0,ifname=tap0,script=no,downscript=no
"""
def make_legacy_pcnet(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x400, 2, mmio=True)
  make_hypercube(s, 0x1821000, 0x20, 0, mmio=True)
  make_hypercube(s, 0xc000, 0x20, 1, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device rtl8139,netdev=net0 
-netdev tap,id=net0,ifname=tap0,script=no,downscript=no
"""
def make_legacy_rtl8139(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x400, 2, mmio=True)
  make_hypercube(s, 0xc000, 0x100, 0, mmio=False)
  make_hypercube(s, 0x1821000, 0x100, 1, mmio=True)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device e1000,netdev=net0 
-netdev tap,id=net0,ifname=tap0,script=no,downscript=no
"""
def make_legacy_e1000(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0xc000, 0x40, 0, mmio=False)
  make_hypercube(s, 0x1821000, 0x20000, 1, mmio=True)


"""
-net none -device e1000 -device ne2k_pci
"""
def make_legacy_ne2000(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0xc000, 0x100, 1, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device i82550,netdev=net0 
-netdev tap,id=net0,ifname=tap0,script=no,downscript=no 2> /dev/null
"""
def make_legacy_ee100pro(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x400, 2, mmio=True)
  make_hypercube(s, 0xc000, 0x40, 0, mmio=False)
  make_hypercube(s, 0x1821000, 0x1000, 1, mmio=True)
  make_hypercube(s, 0x1822000, 0x20000, 3, mmio=True)

"""
qemu-img create sd-card.img 10M
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device sdhci-pci -drive format=raw,file=sd-card.img,if=none,id=disk,cache=writeback,discard=unmap -device sd-card,drive=disk 2> /dev/null
"""
def make_legacy_sdhci(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x400, 2, mmio=True)
  make_hypercube(s, 0x1821000, 0x100, 1, mmio=True)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -serial file:/tmp/A
"""
def make_legacy_serial(s):
  make_globals(s)
  make_hypercube(s, 0x3F8, 0x8, 0, mmio=False)
  make_hypercube(s, 0x2F8, 0x8, 1, mmio=False)
  make_hypercube(s, 0x3E8, 0x8, 2, mmio=False)
  make_hypercube(s, 0x2E8, 0x8, 3, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -parallel file:/tmp/A
"""
def make_legacy_parallel(s):
  make_globals(s)
  make_hypercube(s, 0x378, 0x4, 0, mmio=False)
  make_hypercube(s, 0x3BC, 0x4, 1, mmio=False)
  make_hypercube(s, 0x278, 0x4, 2, mmio=False)

"""
dd if=/dev/zero of=floppy.img bs=1024 count=1440
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -fda floppy.img 
"""
def make_legacy_floppy(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0x3F0, 0x8, 0, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device sb16
"""
def make_legacy_soundblaster(s):
  make_globals(s)
  make_hypercube(s, 0x224, 0x4, 0, mmio=False)
  make_hypercube(s, 0x22a, 0x1, 0, mmio=False)
  make_hypercube(s, 0x22c, 0x4, 0, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device ES1370
"""
def make_legacy_es1370(s):
  make_globals(s)
  make_hypercube(s, 0xc000, 0x100, 0, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device cs4231a
"""
def make_legacy_cs4231a(s):
  make_globals(s)
  make_hypercube(s, 0x534, 0x4, 0, mmio=False)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device intel-hda -device hda-duplex
"""
def make_legacy_intel_hda(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0x1821000, 0x4000, 0, mmio=True)

"""
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -nographic -device AC97
"""
def make_legacy_ac97(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x400, 2, mmio=True)
  make_hypercube(s, 0xc000, 0x100, 1, mmio=False)
  make_hypercube(s, 0xc400, 0x100, 0, mmio=False)

"""
qemu-img create hdd.img 10M
LD_BIND_NOW=1 LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4:./hypertrash_crash_detector_asan ASAN_OPTIONS=log_path=/tmp/data.log:abort_on_error=true:detect_leaks=false 
/home/user/qemu-5.0.0-rc3/x86_64-softmmu/qemu-system-x86_64 -cdrom hypertrash.iso -enable-kvm -m 100 -net none -hda hdd.img
"""
def make_legacy_ide_core(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0x1f0, 0x8, 0, mmio=False)
  make_hypercube(s, 0x376, 0x1, 1, mmio=False)
  make_hypercube(s, 0x170, 0x8, 3, mmio=False)
  make_hypercube(s, 0x3f6, 0x1, 4, mmio=False)  
  make_hypercube(s, 0xc000, 0x10, 5, mmio=False)


def make_legacy_xhci(s):
  make_globals(s)
  make_hypercube_generic(s)
  make_hypercube(s, 0x0, 0x100, 2, mmio=True)
  make_hypercube(s, 0x1821000, 0x4000, 0, mmio=True)
