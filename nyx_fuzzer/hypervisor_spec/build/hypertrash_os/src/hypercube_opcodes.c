/*
 * HyperCube OS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */ 

#include <stdint.h>
#include "../tesseract/mmio.h"
#include "../tesseract/io.h"
#include "libk.h"

/* 
Ported Opcodes:
 - write_mmio 
 - read_mmio
 - xor_mmio
 - memset mmio (missing?)
 - writes mmio (named memset)
 - reads (missing?)
 - exec_mmio_write_scratch_ptr

  ===== io

  - io_write
  - io_read
  - exec_io_xor
  - exec_io_memset
  - exec_io_writes
  - io_reads
  - exec_io_write_scratch_ptr

  ===== ignore
 - ignore: write msr(msr num, mask)
 - ignore: hypercall(eax, ebx, ecx, edx, esi)
 - ignore: vmport(ecx, ebx)
*/


/* 

void exec_mmio_write_8(state_t* state_obj, hexa_op* input){

write_mmio(region_base, region_size, offset, data)
read_mmio(region_base, region_size, offset)

write_io(region_base, region_size, offset, data)
read_io(region_base, region_size, offset)

xor mmio(region_base, region_size, offset, mask)
xor io(region_base, region_size, offset, mask)


*/

uint8_t* tmp_data[0x4000];
uint32_t x32 = 314159265;

static uint32_t xorshift32(){
  x32 ^= x32 << 13;
  x32 ^= x32 >> 17;
  x32 ^= x32 << 5;
  return x32;
}

static void set_seed(uint32_t seed){
	x32 ^= seed;
}

void hc_set_scratch_area(uintptr_t addr, uint16_t offset, uint32_t seed, uint16_t len){
	uint32_t* scatch_area = (uint32_t*)addr;
	len %= 0x1000/4;

	set_seed(seed);

	for(uint32_t i = offset; (i-offset) < len && i < (0x1000/4); i++){
		scatch_area[i] = xorshift32();
	}
}

void hc_set_scratch_area_2(uintptr_t addr, uint32_t seed, uint16_t len){
	uint32_t* scatch_area = (uint32_t*)addr;
	len %= 0x1000/4;

	set_seed(seed);

	for(uint32_t i = 0; i < len && i < (0x1000/4); i++){
		scatch_area[i] = xorshift32();
	}
}

static inline void *__movsb(void *d, const void *s, size_t n) {
#ifdef INTERPRETER_BENCHMARK
	return NULL;
#endif
  asm volatile ("rep movsb"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
  return d;
}

static inline void *__movsw(void *d, const void *s, size_t n) {
#ifdef INTERPRETER_BENCHMARK
	return NULL;
#endif
  asm volatile ("rep movsw"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
  return d;
}

static inline void *__movsl(void *d, const void *s, size_t n) {
#ifdef INTERPRETER_BENCHMARK
	return NULL;
#endif
  asm volatile ("rep movsl"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
  return d;
}


static inline uint8_t safe_inb(uint16_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	uint8_t data = 0;
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"inb %%dx,%%al\n\r"
	
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		:"=a"(data)
		:"d"(port));
	return data;
}

static inline uint16_t safe_inw(uint16_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	uint16_t data = 0;

	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"inw %%dx,%%ax\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:"=a"(data)
		:"d"(port)
		);
	return data;
}

static inline uint32_t safe_inl(uint32_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	uint32_t data = 0;
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"inl %%dx,%%eax\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:"=a"(data)
		:"d"(port)
		);
	return data;
}


static inline void safe_outb(uint8_t data, uint16_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	//if(port == 0x3F8 || port == 0x2F8 || port == 0x3E8 || port == 0x2E8 || port == 0x434 || port == 0x42c|| port == 0x445|| port ==0x432)
	//	return;
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"outb %%al,  %%dx\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:"=a"(data):"d"(port)
		);
}

static inline void safe_outw(uint16_t data, uint16_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	//if(port == 0x3F8 || port == 0x2F8 || port == 0x3E8 || port == 0x2E8|| port == 0x434|| port == 0x42c|| port == 0x445|| port ==0x432)
	//	return;
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"outw %%ax,  %%dx\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:"=a"(data):"d"(port)
		);
}

static inline void safe_outl(uint32_t data, uint16_t port){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	//if(port == 0x3F8 || port == 0x2F8 || port == 0x3E8 || port == 0x2E8|| port == 0x434|| port == 0x42c|| port == 0x445|| port ==0x432)
	//	return;
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"outl %%eax,  %%dx\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:"=a"(data):"d"(port)
		);
}


static void exec_mmio_write(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t size){		
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

  switch(size){
    case 0:
        _mmio_write8(_offset, region_base, data);
        break;
    case 1:
        _mmio_write16(_offset, region_base, data);
        break;
    case 2:
        _mmio_write32(_offset, region_base, data);
        break;
    default:
        abort();
	}
	asm volatile("" ::: "memory");
}

void hc_mmio_write_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data){
	exec_mmio_write(region_base, region_size, offset, data, 2);
}

void hc_mmio_write_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data){
	exec_mmio_write(region_base, region_size, offset, (uint32_t)data, 1);
}

void hc_mmio_write_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data){
	exec_mmio_write(region_base, region_size, offset, (uint32_t)data, 0);
}


static void exec_mmio_read(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t size){
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}
  switch(size){
    case 0:
        _mmio_read8(_offset, region_base);
        break;
    case 1:
        _mmio_read16(_offset, region_base);
        break;
    case 2:
        _mmio_read32(_offset, region_base);
        break;
    default:
        abort();
	}
	asm volatile("" ::: "memory");
}

void hc_mmio_read_32(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_mmio_read(region_base, region_size, offset, 2);
}

void hc_mmio_read_16(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_mmio_read(region_base, region_size, offset, 1);
}

void hc_mmio_read_8(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_mmio_read(region_base, region_size, offset, 0);
}


static void exec_mmio_xor(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask, uint8_t size){		
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}
  switch(size){
    case 0:        					
        _mmio_write8(_offset, region_base, ((uint8_t)mask) ^ _mmio_read8(_offset, region_base));
        break;
    case 1:
        _mmio_write16(_offset, region_base, ((uint16_t)mask) ^ _mmio_read16(_offset, region_base));
        break;
    case 2:
        _mmio_write32(_offset, region_base, ((uint32_t)mask) ^ _mmio_read32(_offset, region_base));
        break;
    default:
        abort();
	}
	asm volatile("" ::: "memory");
}

void hc_mmio_xor_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask){
	exec_mmio_xor(region_base, region_size, offset, mask, 2);
}

void hc_mmio_xor_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t mask){
	exec_mmio_xor(region_base, region_size, offset, (uint32_t)mask, 1);
}

void hc_mmio_xor_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t mask){
	exec_mmio_xor(region_base, region_size, offset, (uint32_t)mask, 0);
}


static void exec_mmio_write_bruteforce(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num, uint8_t size){		
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}
  switch(size){
    case 0:
        for(uint32_t i = 0; i < num; i++){
          _mmio_write8(_offset, region_base, (data + i) % 0xFF);
        }
        break;
    case 1:
        for(uint32_t i = 0; i < num; i++){
          _mmio_write16(_offset, region_base, (data + i) % 0xFFFF);
        }
        break;
    case 2:
        for(uint32_t i = 0; i < num; i++){
          _mmio_write32(_offset, region_base, (data + i) % 0xFFFFFFFF);
        }
        break;
    default:
        assert2(false, "exec_mmio_write_bruteforce");
	}
	asm volatile("" ::: "memory");
}

void hc_mmio_write_bruteforce_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num){
	exec_mmio_write_bruteforce(region_base, region_size, offset, data, num, 2);
}

void hc_mmio_write_bruteforce_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint8_t num){
	exec_mmio_write_bruteforce(region_base, region_size, offset, (uint32_t)data, num, 1);
}

void hc_mmio_write_bruteforce_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint8_t num){
	exec_mmio_write_bruteforce(region_base, region_size, offset, (uint32_t)data, num, 0);
}


static void exec_mmio_memset(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num, uint8_t size){
	uint32_t _offset = 0;
	if(region_size <= 4){
		return;
	}
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}
	int32_t memset_size = (num%0x1000);

	if(memset_size+(1<<size) >= (region_size-_offset)){
		memset_size = (region_size-_offset) -(1<<size);
		if (memset_size < 0){
			return;
		}
	}

	switch(size){
		case 0:
				for(uint32_t i = 0; i < memset_size; i++){
					((uint8_t*)tmp_data)[i] = (uint8_t)data;
				}
				__movsb((void*)(region_base+_offset), tmp_data, memset_size);
				break;
		case 1:
				for(uint32_t i = 0; i < memset_size/2; i++){
					((uint16_t*)tmp_data)[i] = (uint16_t)data;
				}
				__movsw((void*)(region_base+_offset), tmp_data, memset_size/2);
				break;
		case 2:
				for(uint32_t i = 0; i < memset_size/4; i++){
					((uint32_t*)tmp_data)[i] = (uint32_t)data;
				}
				__movsl((void*)(region_base+_offset), tmp_data, memset_size/4);
				break;
		default:
				abort();
	}
	asm volatile("" ::: "memory");
}

void hc_mmio_memset_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num){
	exec_mmio_memset(region_base, region_size, offset, data, num, 2);
}

void hc_mmio_memset_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num){
	exec_mmio_memset(region_base, region_size, offset, (uint32_t)data, num, 1);
}

void hc_mmio_memset_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint16_t num){
	exec_mmio_memset(region_base, region_size, offset, (uint32_t)data, num, 0);
}


void hc_mmio_write_scratch_ptr(uint32_t region_base, uint32_t region_size, uint32_t offset){
	uint32_t _offset = 0;
	if(region_size-(1<<2) > 0){
  	_offset = offset % (region_size-(1<<2));
	}
	else{
		_offset = offset % region_size;
	}
	
	_mmio_write32(region_base, _offset, 0);

}

static void exec_io_write(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t size){		
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

  switch(size){
    case 0:
				safe_outb(data, _offset+region_base);
        break;
    case 1:
				safe_outw(data, _offset+region_base);
        break;
    case 2:
				safe_outl(data, _offset+region_base);
        break;
    default:
        assert2(false, "io_write");
	}
	asm volatile("" ::: "memory");
}

void hc_io_write_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data){
	exec_io_write(region_base, region_size, offset, data, 2);
}

void hc_io_write_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data){
	exec_io_write(region_base, region_size, offset, (uint32_t)data, 1);
}

void hc_io_write_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data){
	exec_io_write(region_base, region_size, offset, (uint32_t)data, 0);
}


static void exec_io_read(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t size){
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

  switch(size){
    case 0:
				safe_inb(region_base+_offset);
        break;
    case 1:
				safe_inw(region_base+_offset);
        break;
    case 2:
				safe_inl(region_base+_offset);
        break;
    default:
        assert2(false, "io_read");
	}
	asm volatile("" ::: "memory");
}

void hc_io_read_32(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_io_read(region_base, region_size, offset, 2);
}

void hc_io_read_16(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_io_read(region_base, region_size, offset, 1);
}

void hc_io_read_8(uint32_t region_base, uint32_t region_size, uint32_t offset){
	exec_io_read(region_base, region_size, offset, 0);
}

static void exec_io_xor(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask, uint8_t size){		
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

  switch(size){
    case 0:       
				safe_outb(mask ^ safe_inb(_offset+region_base), _offset+region_base);
        break;
    case 1:
				safe_outw(mask ^ safe_inb(_offset+region_base), _offset+region_base);
        break;
    case 2:
				safe_outl(mask ^ safe_inb(_offset+region_base), _offset+region_base);
        break;
    default:
        assert2(false, "io_xor");
	}
	asm volatile("" ::: "memory");
}

void hc_io_xor_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t mask){
	exec_io_xor(region_base, region_size, offset, mask, 2);
}

void hc_io_xor_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t mask){
	exec_io_xor(region_base, region_size, offset, (uint32_t)mask, 1);
}

void hc_io_xor_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t mask){
	exec_io_xor(region_base, region_size, offset, (uint32_t)mask, 0);
}

static void exec_io_write_bruteforce(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num, uint8_t size){
	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

  switch(size){
    case 0:
        for(uint32_t i = 0; i < num; i++){
					safe_outb((data + i) % 0xFF, _offset+region_base);
        }
        break;
    case 1:
        for(uint32_t i = 0; i < num; i++){
					safe_outw((data + i) % 0xFFFF, _offset+region_base);
        }
        break;
    case 2:
        for(uint32_t i = 0; i < num; i++){
					safe_outl((data + i) % 0xFFFFFFFF, _offset+region_base);
        }
        break;
    default:
        assert2(false, "io_bruteforce");
	}
	asm volatile("" ::: "memory");
}

void hc_io_write_bruteforce_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num){
	exec_io_write_bruteforce(region_base, region_size, offset, data, num, 2);
}

void hc_io_write_bruteforce_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint8_t num){
	exec_io_write_bruteforce(region_base, region_size, offset, (uint32_t)data, num, 1);
}

void hc_io_write_bruteforce_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint8_t num){
	exec_io_write_bruteforce(region_base, region_size, offset, (uint32_t)data, num, 0);
}


void hc_io_write_scratch_ptr(uint32_t region_base, uint32_t region_size, uint32_t offset){
	uint32_t _offset = 0;
	if(region_size-(1<<2) != 0){
  	_offset = offset % (region_size-(1<<2));
	}

	safe_outl(0, _offset+region_base);
}

static void exec_io_memset(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint8_t num, uint16_t size){
  if((1<<size) >= region_size){
    return;
  }

	uint32_t _offset = 0;
	if(region_size-(1<<size) != 0){
  	_offset = offset % (region_size-(1<<size));
	}

	switch(size){
		case 0:
				for(uint32_t i = 0; (_offset+region_base+(i*1)+0) < region_base+region_size; i++){
					safe_outb(data, _offset+region_base+(i*1));
				}
				break;
		case 1:
				for(uint32_t i = 0; (_offset+region_base+(i*2)+1) < region_base+region_size; i++){
					safe_outw(data, _offset+region_base+(i*2));
				}
				break;
		case 2:
				for(uint32_t i = 0; (_offset+region_base+(i*4)+3) < region_base+region_size; i++){
					safe_outl(data, _offset+region_base+(i*4));
				}
				break;
		default:
				assert2(false, "io_memset");
	}
	asm volatile("" ::: "memory");
}

void hc_io_memset_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint32_t data, uint16_t num){
	exec_io_memset(region_base, region_size, offset, data, num, 2);
}

void hc_io_memset_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num){
	exec_io_memset(region_base, region_size, offset, (uint32_t)data, num, 1);
}

void hc_io_memset_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t data, uint16_t num){
	exec_io_memset(region_base, region_size, offset, (uint32_t)data, num, 0);
}

static inline void safe_rep_insb(uint16_t count, uint16_t port, uint32_t data){		
		asm volatile("pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;" ::);
		
		asm volatile("rep insb" : : "c"(count), "d"(port), "D"(data));
	
		asm volatile("popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;" ::);
}

static void exec_io_reads(uint32_t region_base, uint32_t region_size, uint32_t offset, uint8_t num, uint16_t size){
	if(region_size < (size << 1))
		return;
	uint32_t _offset = offset % (region_size - (size<<1) + 1);
	int32_t memset_size = num%256;

	switch(size){
		case 0:
				asm("rep insb" : : "c"(memset_size), "d"(_offset+region_base), "D"(tmp_data));
				break;
		case 1:
				asm("rep insw" : : "c"(memset_size), "d"(_offset+region_base), "D"(tmp_data));
				break;
		case 2:
				asm("rep insl" : : "c"(memset_size), "d"(_offset+region_base), "D"(tmp_data));
				break;
		default:
				assert2(false, "io_memset");
	}
	asm volatile("" ::: "memory");
}

void hc_io_reads_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num){
	exec_io_reads(region_base, region_size, offset, num, 2);
}

void hc_io_reads_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num){
	exec_io_reads(region_base, region_size, offset, num, 1);
}

void hc_io_reads_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t num){
	exec_io_reads(region_base, region_size, offset, num, 0);
}

static void exec_io_writes(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint8_t num, uint16_t size){
	if(region_size < (size << 1))
		return;

	uint32_t _offset = offset % (region_size - (size<<1) + 1);  
	int32_t memset_size = num%256;

	switch(size){
		case 0:
				asm("rep outsb" : : "c"(memset_size), "d"(_offset+region_base), "D"(data%0x400));
				break;
		case 1:
				asm("rep outsw" : : "c"(memset_size/2), "d"(_offset+region_base), "D"(data%0x400));
				break;
		case 2:
				asm("rep outsl" : : "c"(memset_size/4), "d"(_offset+region_base), "D"(data%0x400));
				break;
		default:
				assert2(false, "io_memset");
	}
	asm volatile("" ::: "memory");
}

void hc_io_writes_32(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num){
	exec_io_writes(region_base, region_size, offset, data, num, 2);
}

void hc_io_writes_16(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num){
	exec_io_writes(region_base, region_size, offset, data, num, 1);
}

void hc_io_writes_8(uint32_t region_base, uint32_t region_size, uint32_t offset, uint16_t data, uint16_t num){
	exec_io_writes(region_base, region_size, offset, data, num, 0);
}