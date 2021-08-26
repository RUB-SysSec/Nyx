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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "state.h"
#include "mmio.h"
#include "io.h"
#include "opcodes.h"

#include "dict.h"

#define UNUSED(x) (void)(x)


#ifdef HYPERTRASH
#include "tty.h"
#include "msr.h"

#else

void rdmsr32(uint32_t msr, uint32_t *lo, uint32_t *hi){
	UNUSED(msr);
	UNUSED(lo);
	UNUSED(hi);
}
void wrmsr32(uint32_t msr, uint32_t lo, uint32_t hi){
	UNUSED(msr);
	UNUSED(lo);
	UNUSED(hi);
}

void set_ret(void* addr){
	UNUSED(addr);
}

#endif


extern void set_ret(void* addr);


uint8_t* tmp_data[0x1000];

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

static void exec_mmio_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_write_8* op = (op_mmio_write_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_write8(offset, base, ((op_mmio_write_8*)op)->data);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_write16(offset, base, ((op_mmio_write_16*)op)->data);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_write32(offset, base, ((op_mmio_write_32*)op)->data);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_write_32(state_t* state_obj, hexa_op* input){
	exec_mmio_write(state_obj, input, 2);
}

void exec_mmio_write_16(state_t* state_obj, hexa_op* input){
	exec_mmio_write(state_obj, input, 1);
}

void exec_mmio_write_8(state_t* state_obj, hexa_op* input){
	exec_mmio_write(state_obj, input, 0);
}

static void exec_io_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_write_8* op = (op_io_write_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;

		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//printf("IO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_io_write_8*)op)->data);
					safe_outb(((op_io_write_8*)op)->data, offset+base);
					break;
			case 1: //printf("IO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_io_write_16*)op)->data);
					safe_outw(((op_io_write_16*)op)->data, offset+base);
					break;
			case 2: //printf("IO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_io_write_32*)op)->data);
					safe_outl(((op_io_write_32*)op)->data, offset+base);
					break;
			default:
					abort();
		}
	}
}

void exec_io_write_32(state_t* state_obj, hexa_op* input){
	exec_io_write(state_obj, input, 2);
}

void exec_io_write_16(state_t* state_obj, hexa_op* input){
	exec_io_write(state_obj, input, 1);
}

void exec_io_write_8(state_t* state_obj, hexa_op* input){
	exec_io_write(state_obj, input, 0);
}

void exec_mmio_write_scratch_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, base;
	op_mmio_write_scratch_ptr* op = (op_mmio_write_scratch_ptr*) input;

	if(state_obj->num_alloc_areas && state_obj->num_mmio_areas){
	
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;

		//offset = state_obj->a[0] % 0x1000;
		offset = op->offset % (state_obj->mmio_area[slot]->size-8);

		//mmio_write32(offset, base, data);
		//mmio_write32(offset, base, input+sizeof(op));
		_mmio_write32(base, offset, (uint32_t)(input+sizeof(op)));
		/* Aligned */
		//mmio_write32(offset-(offset%4)E1000_TDH, base, data);
		//mmio_write32(0x03810, base, 0x0);
	}
	asm volatile("" ::: "memory");
}

void exec_io_write_scratch_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, base;
	op_io_write_scratch_ptr* op = (op_io_write_scratch_ptr*) input;



	if(state_obj->num_alloc_areas && state_obj->num_io_areas){
	
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;

		offset = op->offset % (state_obj->io_area[slot]->size);//-4);

		//outl(data, offset+base);
		safe_outl((uint32_t)(input+sizeof(op)), offset+base);
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

static void exec_mmio_memset(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base, memset_size;
	op_mmio_memset_8* op = (op_mmio_memset_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;

		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));
		memset_size = (op->size % 0x1000) % (state_obj->mmio_area[slot]->size - offset);


		switch(size){
			case 0:	//printf("MMIO MEMSET8(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_8*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size; i++){
						((uint8_t*)tmp_data)[i] = (((op_mmio_memset_8*)op)->data);
					}
					__movsb((void*)(base+offset), tmp_data, memset_size);
					break;

			case 1:	//printf("MMIO MEMSET16(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_16*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size/2; i++){
						((uint16_t*)tmp_data)[i] = ((op_mmio_memset_16*)op)->data;
					}
					__movsw((void*)(base+offset), tmp_data, memset_size/2);
					break;
			case 2:	//printf("MMIO MEMSET32(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_32*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size/4; i++){
						((uint32_t*)tmp_data)[i] = ((op_mmio_memset_32*)op)->data;
					}
					__movsl((void*)(base+offset), tmp_data, memset_size/4);
					break;;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_memset_32(state_t* state_obj, hexa_op* input){
	exec_mmio_memset(state_obj, input, 2);
}

void exec_mmio_memset_16(state_t* state_obj, hexa_op* input){
	exec_mmio_memset(state_obj, input, 1);
}

void exec_mmio_memset_8(state_t* state_obj, hexa_op* input){
	exec_mmio_memset(state_obj, input, 0);
}

static void exec_io_memset(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base, memset_size;
	op_io_memset_8* op = (op_io_memset_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;

		offset = op->offset % (state_obj->io_area[slot]->size);//-(1<<size));
		memset_size = (op->size % 0xff) % (state_obj->io_area[slot]->size);// - offset);

		switch(size){
			case 0:	//printf("IO MEMSET 8(%x + %x, %x, %x)\n\r", offset, base, ((op_io_write_8*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size; i++){
						safe_outb(((op_io_write_8*)op)->data, offset+(i*1)+base);
					}
					break;

			case 1: //printf("IO MEMSET 16(%x + %x, %x, %x)\n\r", offset, base, ((op_io_write_16*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size/2; i++){
						safe_outw(((op_io_write_16*)op)->data, offset+(i*2)+base);
					}
					break;
			case 2: //printf("IO MEMSET 32(%x + %x, %x, %x)\n\r", offset, base, ((op_io_write_32*)op)->data, memset_size);
					for(uint32_t i = 0; i < memset_size/4; i++){
						safe_outl(((op_io_write_32*)op)->data, offset+(i*4)+base);
					}
					break;;
			default:
					abort();
		}
	}
}


void exec_io_memset_32(state_t* state_obj, hexa_op* input){
	exec_io_memset(state_obj, input, 2);
}

void exec_io_memset_16(state_t* state_obj, hexa_op* input){
	exec_io_memset(state_obj, input, 1);
}

void exec_io_memset_8(state_t* state_obj, hexa_op* input){
	exec_io_memset(state_obj, input, 0);
}

static void exec_mmio_xor(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_xor_8* op = (op_mmio_xor_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	//printf("MMIO XOR 8(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_32*)op)->data);
					_mmio_write8(offset, base, ((op_mmio_xor_8*)op)->data ^ _mmio_read8(offset, base));
					break;
			case 1: //printf("MMIO XOR 16(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_32*)op)->data);
					_mmio_write16(offset, base, ((op_mmio_xor_16*)op)->data ^ _mmio_read16(offset, base));
					break;
			case 2: //printf("MMIO XOR 32(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_32*)op)->data);
					_mmio_write32(offset, base, ((op_mmio_xor_32*)op)->data ^ _mmio_read32(offset, base));
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_xor_32(state_t* state_obj, hexa_op* input){
	exec_mmio_xor(state_obj, input, 2);
}

void exec_mmio_xor_16(state_t* state_obj, hexa_op* input){
	exec_mmio_xor(state_obj, input, 1);
}

void exec_mmio_xor_8(state_t* state_obj, hexa_op* input){
	exec_mmio_xor(state_obj, input, 0);
}

static void exec_io_xor(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_xor_8* op = (op_io_xor_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;

		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0: //printf("IO XOR 8(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_8*)op)->data);
					safe_outb(((op_io_xor_8*)op)->data ^ safe_inb(offset+base), offset+base);
					break;
			case 1: //printf("IO XOR 8(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_16*)op)->data);
					safe_outw(((op_io_xor_16*)op)->data ^ safe_inw(offset+base), offset+base);
					break;
			case 2: //printf("IO XOR 8(%x, %x, %x)\n\r", offset, base, ((op_mmio_xor_32*)op)->data);
					safe_outl(((op_io_xor_32*)op)->data ^ safe_inl(offset+base), offset+base);
					break;
			default:
					abort();
		}
	}
}

void exec_io_xor_32(state_t* state_obj, hexa_op* input){
	exec_io_xor(state_obj, input, 2);	
}

void exec_io_xor_16(state_t* state_obj, hexa_op* input){
	exec_io_xor(state_obj, input, 1);	
}

void exec_io_xor_8(state_t* state_obj, hexa_op* input){
	exec_io_xor(state_obj, input, 0);	
}

static void exec_mmio_write_bruteforce(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_bruteforce_8* op = (op_mmio_bruteforce_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	//vprintf("MMIO BRUTEFORCE8(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_8*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						_mmio_write8(offset, base, (((op_mmio_bruteforce_8*)op)->data + i) % 0xFF);
					}						
					break;
			case 1: //printf("MMIO BRUTEFORCE16(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_16*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						_mmio_write16(offset, base, (((op_mmio_bruteforce_16*)op)->data + i) % 0xFFFF);
					}
					break;
			case 2: //printf("MMIO BRUTEFORCE32(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_32*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						_mmio_write32(offset, base, (((op_mmio_bruteforce_32*)op)->data + i) % 0xFFFFFFFF);
					}
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_write_bruteforce_32(state_t* state_obj, hexa_op* input){
	exec_mmio_write_bruteforce(state_obj, input, 2);	
}

void exec_mmio_write_bruteforce_16(state_t* state_obj, hexa_op* input){
	exec_mmio_write_bruteforce(state_obj, input, 1);	
}

void exec_mmio_write_bruteforce_8(state_t* state_obj, hexa_op* input){
	exec_mmio_write_bruteforce(state_obj, input, 0);	
}


static void exec_mmio_dict_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_write_dict_8* op = (op_mmio_write_dict_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		//printf("SIZE: %d\n", dict_size());
		offset = dict[op->offset % dict_size()];
		//printf("OFFSET: %x %d\n", offset, op->offset % dict_size());
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_write8(offset, base, ((op_mmio_write_dict_8*)op)->data);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_write16(offset, base, ((op_mmio_write_dict_16*)op)->data);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_write32(offset, base, ((op_mmio_write_dict_32*)op)->data);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_write_dict_32(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write(state_obj, input, 2);
}

void exec_mmio_write_dict_16(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write(state_obj, input, 1);
}

void exec_mmio_write_dict_8(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write(state_obj, input, 0);
}

void exec_mmio_write_scratch_dict_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, base, data;
	op_mmio_write_scratch_dict_ptr* op = (op_mmio_write_scratch_dict_ptr*) input;

	if(state_obj->num_alloc_areas && state_obj->num_mmio_areas){
	
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;

		//offset = state_obj->a[0] % 0x1000;
		//offset = op->offset % (state_obj->mmio_area[slot]->size-4);
		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->mmio_area[slot]->size-(1<<2));

		data = ((uint32_t)(state_obj->alloc_areas[op->s_slot % state_obj->num_alloc_areas] + op->s_offset)) % 0x1000;

		data = 0xfebd1000;
		//mmio_write32(offset, base, data);

		/* Aligned */
		_mmio_write32(offset, base, data);
	}
	asm volatile("" ::: "memory");
}


static void exec_io_write_dict(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_write_dict_8* op = (op_io_write_dict_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;

		//offset = op->offset % (state_obj->io_area[slot]->size);

		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//printf("IO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_io_write_8*)op)->data);
					safe_outb(((op_io_write_dict_8*)op)->data, offset+base);
					break;
			case 1: //printf("IO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_io_write_16*)op)->data);
					safe_outw(((op_io_write_dict_16*)op)->data, offset+base);
					break;
			case 2: //printf("IO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_io_write_32*)op)->data);
					safe_outl(((op_io_write_dict_32*)op)->data , offset+base);
					break;
			default:
					abort();
		}
	}
}

void exec_io_write_dict_32(state_t* state_obj, hexa_op* input){
	exec_io_write_dict(state_obj, input, 2);
}

void exec_io_write_dict_16(state_t* state_obj, hexa_op* input){
	exec_io_write_dict(state_obj, input, 1);
}

void exec_io_write_dict_8(state_t* state_obj, hexa_op* input){
	exec_io_write_dict(state_obj, input, 0);
}


static void exec_mmio_read_dict(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_read_dict* op = (op_mmio_read_dict*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		//printf("SIZE: %d\n", sizeof(dict)/4);
		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_read8(offset, base);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_read16(offset, base);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_read32(offset, base);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_read_dict_32(state_t* state_obj, hexa_op* input){
	exec_mmio_read_dict(state_obj, input, 2);
}

void exec_mmio_read_dict_16(state_t* state_obj, hexa_op* input){
	exec_mmio_read_dict(state_obj, input, 1);
}

void exec_mmio_read_dict_8(state_t* state_obj, hexa_op* input){
	exec_mmio_read_dict(state_obj, input, 0);
}

static void exec_io_write_bruteforce(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_bruteforce_8* op = (op_io_bruteforce_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//vprintf("MMIO BRUTEFORCE8(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_8*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						safe_outb((((op_io_bruteforce_8*)op)->data + i) % 0xFF, offset+base);
					}						
					break;
			case 1: //printf("MMIO BRUTEFORCE16(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_16*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						safe_outb((((op_io_bruteforce_16*)op)->data + i) % 0xFFFF, offset+base);
					}
					break;
			case 2: //printf("MMIO BRUTEFORCE32(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_32*)op)->data, op->size);
					for(uint32_t i = 0; i < op->size; i++){
						safe_outl((((op_io_bruteforce_32*)op)->data + i) % 0xFFFFFFFF, offset+base);
					}
					break;
			default:
					abort();
		}
	}
}

void exec_io_write_bruteforce_32(state_t* state_obj, hexa_op* input){
	exec_io_write_bruteforce(state_obj, input, 2);	
}


void exec_io_write_bruteforce_16(state_t* state_obj, hexa_op* input){
	exec_io_write_bruteforce(state_obj, input, 1);	
}

void exec_io_write_bruteforce_8(state_t* state_obj, hexa_op* input){
	exec_io_write_bruteforce(state_obj, input, 0);	
}

static void exec_io_reads(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_reads_8* op = (op_io_reads_8*) input;


	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//vprintf("MMIO BRUTEFORCE8(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_8*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep insb" : : "c"(op->size%0x400), "d"(offset+base), "D"(tmp_data));
					//for(uint32_t i = 0; i < op->size; i++){
					//	outb(((op_io_bruteforce_8*)op)->data % 0xFF, offset+base);
					//}						
					break;
			case 1: //printf("MMIO BRUTEFORCE16(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_16*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep insw" : : "c"(op->size%0x400), "d"(offset+base), "D"(tmp_data));

					//for(uint32_t i = 0; i < op->size; i++){
					//	outb(((op_io_bruteforce_16*)op)->data % 0xFFFF, offset+base);
					//}
					break;
			case 2: //printf("MMIO BRUTEFORCE32(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_32*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep insl" : : "c"(op->size%0x400), "d"(offset+base), "D"(tmp_data));
					//for(uint32_t i = 0; i < op->size; i++){
					//	outl(((op_io_bruteforce_32*)op)->data % 0xFFFFFFFF, offset+base);
					//}
					break;
			default:
					abort();
		}
	}
}

void exec_io_reads_32(state_t* state_obj, hexa_op* input){
	exec_io_reads(state_obj, input, 2);	
}


void exec_io_reads_16(state_t* state_obj, hexa_op* input){
	exec_io_reads(state_obj, input, 1);	
}

void exec_io_reads_8(state_t* state_obj, hexa_op* input){
	exec_io_reads(state_obj, input, 0);	
}

static void exec_io_writes(state_t* state_obj, hexa_op* input, uint8_t size){
	//printf("%s\n", __func__);
	uint32_t offset, slot, base;
	op_io_writes_8* op = (op_io_writes_8*) input;

	return; /* TODO FIX THIS */

	//uint8_t buffer[256];

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//vprintf("MMIO BRUTEFORCE8(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_8*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep outsb" : : "c"(op->size%0x400), "d"(offset+base), "D"(0x1000));
					//for(uint32_t i = 0; i < op->size; i++){
					//	outb(((op_io_bruteforce_8*)op)->data % 0xFF, offset+base);
					//}						
					break;
			case 1: //printf("MMIO BRUTEFORCE16(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_16*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep outsw" : : "c"(op->size%0x400), "d"(offset+base), "D"(0x1000));

					//for(uint32_t i = 0; i < op->size; i++){
					//	outb(((op_io_bruteforce_16*)op)->data % 0xFFFF, offset+base);
					//}
					break;
			case 2: //printf("MMIO BRUTEFORCE32(%x + %x, %x, %x)\n", offset, base, ((op_mmio_memset_32*)op)->data, op->size);
					#ifdef INTERPRETER_BENCHMARK
						return 0;
					#endif
					asm("rep outsl" : : "c"(op->size%0x400), "d"(offset+base),  "D"(0x1000));
					//for(uint32_t i = 0; i < op->size; i++){
					//	outl(((op_io_bruteforce_32*)op)->data % 0xFFFFFFFF, offset+base);
					//}
					break;
			default:
					abort();
		}
	}
}

void exec_io_writes_32(state_t* state_obj, hexa_op* input){
	exec_io_writes(state_obj, input, 2);	
}


void exec_io_writes_16(state_t* state_obj, hexa_op* input){
	exec_io_writes(state_obj, input, 1);	
}

void exec_io_writes_8(state_t* state_obj, hexa_op* input){
	exec_io_writes(state_obj, input, 0);	
}


static void exec_mmio_read(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_read* op = (op_mmio_read*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_read8(offset, base);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_read16(offset, base);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_read32(offset, base);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_read_32(state_t* state_obj, hexa_op* input){
	exec_mmio_read(state_obj, input, 2);
}

void exec_mmio_read_16(state_t* state_obj, hexa_op* input){
	exec_mmio_read(state_obj, input, 1);
}

void exec_mmio_read_8(state_t* state_obj, hexa_op* input){
	exec_mmio_read(state_obj, input, 0);
}



static void exec_io_read(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_io_read* op = (op_io_read*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		base = state_obj->io_area[slot]->base;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					//mmio_read8(offset, base);
					safe_inb(base+offset);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					//mmio_read16(offset, base);
					safe_inw(base+offset);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					//mmio_read32(offset, base);
					safe_inl(base+offset);
					break;
			default:
					abort();
		}
	}
}

void exec_io_read_32(state_t* state_obj, hexa_op* input){
	exec_io_read(state_obj, input, 2);
}

void exec_io_read_16(state_t* state_obj, hexa_op* input){
	exec_io_read(state_obj, input, 1);
}

void exec_io_read_8(state_t* state_obj, hexa_op* input){
	exec_io_read(state_obj, input, 0);
}

static inline void static_vmport_in(uint32_t ecx, uint32_t ebx){
	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"movl $0x564D5868, %%eax\n\r"
		"movl $0x5658, %%edx\n\r"
		"movl %0, %%ecx\n\r;"
		"movl %1, %%ebx\n\r;"
		"inl  %%dx, %%eax;\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:
		:"c"(ecx), 
		 "d"(ebx)
		);
}

void exec_vmport_in(state_t* state_obj, hexa_op* input){
	UNUSED(state_obj);	
	vmport_in* op = (vmport_in*) input;

	//printf("%s\n", __func__);
	//uint32_t l_ecx = op->ecx % 0x2c;
	//uint32_t h_ecx = op->ecx >> 16;
	//printf("%s, %x %x %x\n", __func__, l_ecx, h_ecx, op->ebx);

	static_vmport_in(op->ecx, op->ebx);
}

void exec_vmport_in_scratch_ptr(state_t* state_obj, hexa_op* input){
	UNUSED(state_obj);
	vmport_in_scratch_ptr* op = (vmport_in_scratch_ptr*) input;
	//printf("%s\n", __func__);
	//uint32_t l_ecx = op->ecx % 0x2c;
	//uint32_t h_ecx = op->ecx >> 16;
	//printf("%s, %x %x %x\n", __func__, l_ecx, h_ecx, 0x1000);

	static_vmport_in(op->ecx, 0x1000);

}

static inline void kvm_exec_hc(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi){
	UNUSED(edx);

	asm volatile(
		
		"pushl %%eax\n\r;"
		"pushl %%ebx\n\r;"
		"pushl %%ecx\n\r;"
		"pushl %%edx\n\r;"
		"pushl %%esi\n\r;"
		"pushl %%edi\n\r;"
		
		"movl %0, %%eax\n\r"
		"movl %1, %%ebx\n\r"
		"movl %2, %%ecx\n\r"
		"movl %3, %%edx\n\r"
		"movl %4, %%esi\n\r"

		"vmcall\n\r"
		
		"popl %%edi\n\r;"
		"popl %%esi\n\r;"
		"popl %%edx\n\r;"
		"popl %%ecx\n\r;"
		"popl %%ebx\n\r;"
		"popl %%eax\n\r;"
		
		:
		:"r"(eax), 
		 "r"(ebx),
		 "r"(ecx), 
		 "r"(ebx),
		 "r"(esi)
	);
}

void exec_kvm_hypercall(state_t* state_obj, hexa_op* input){
	UNUSED(state_obj);
	kvm_hypercall* op = (kvm_hypercall*) input;
	kvm_exec_hc((op->eax%10) +1, op->ebx, op->ecx, op->edx, op->esi);
}

uint32_t msr_kvm_dict[] = {
	0x4b564d00, //MSR_KVM_WALL_CLOCK_NEW
	0x4b564d01, //MSR_KVM_SYSTEM_TIME_NEW
	0x11, //MSR_KVM_WALL_CLOCK
	0x4b564d02, //MSR_KVM_ASYNC_PF_EN
	0x4b564d03, //MSR_KVM_STEAL_TIME
	0x4b564d04, //MSR_KVM_EOI_EN
	0x000000c1,
0x000000c2,
0x000000cd,
0x000000fe,
0x00000119,
0x00000174,
0x00000175,
0x00000176,
0x00000179,
0x0000017a,
0x0000017b,
0x000003f1,
0x00000600,
0x00000345,
0x00000250,
0x00000258,
0x00000259,
0x00000268,
0x00000269,
0x0000026a,
0x0000026b,
0x0000026c,
0x0000026d,
0x0000026e,
0x0000026f,
0x000002ff,
0x000001d9,
0x000001db,
0x000001dc,
0x000001dd,
0x000001de,
0x00000400,
0x00000401,
0x00000402,
0x00000403,
0x000000c1,
0x000000c2,
0x00000186,
0x00000187,
0xc0011030,
0xc0011031,
0xc0011032,
0xc0011033,
0xc0011034,
0xc0011035,
0xc0011036,
0xc0011037,
0xc0011038,
0xc0011039,
0xc001103a,
0xc001001a,
0xc001001d,
0xc0010010,
0xc0010015,
0xc0010055,
0x00040000,
0x00080000,
0x18181818,
0xc0010000,
0xc0010004,
0xc0010001,
0xc0010005,
0xc0010002,
0xc0010006,
0xc0010003,
0xc0010007,
0xc001001b,
0xc0010015,
0xc0010041,
0xc0010042,
0xc0000080,
0xc0000081,
0xc0000082,
0xc0000085,
0xc0000086,
0xc0000087,
0xc0000088,
0x00000107,
0x00000108,
0x00000109,
0x0000010a,
0x00000110,
0x00000111,
0x00000112,
0x00000113,
0x00000114,
0x00000115,
0x00000116,
0x00000117,
0x00000120,
0x00001107,
0x0000110a,
0x0000110b,
0x00001147,
0x80868010,
0x80868011,
0x80868018,
0x8086801a,
0x00000000,
0x00000001,
0x00000010,
0x00000017,
0x0000002a,
0x0000001b,
0x00000079,
0x0000008b,
0x00000198,
0x00000199,
0x000000e7,
0x000000e8,
0x0000019a,
0x0000019b,
0x0000019c,
0x000001a0,
0x00000186,
0x00000187,
0x00000180,
0x00000181,
0x00000182,
0x00000183,
0x00000184,
0x00000185,
0x00000186,
0x00000187,
0x00000188,
0x00000189,
0x0000018a,
0x00000300,
0x00000301,
0x00000302,
0x00000303,
0x00000304,
0x00000305,
0x00000306,
0x00000307,
0x00000308,
0x00000309,
0x0000030a,
0x0000030b,
0x0000030c,
0x0000030d,
0x0000030e,
0x0000030f,
0x00000310,
0x00000311,
0x00000360,
0x00000361,
0x00000362,
0x00000363,
0x00000364,
0x00000365,
0x00000366,
0x00000367,
0x00000368,
0x00000369,
0x0000036a,
0x0000036b,
0x0000036c,
0x0000036d,
0x0000036e,
0x0000036f,
0x00000370,
0x00000371,
0x000003ca,
0x000003cb,
0x000003b2,
0x000003b3,
0x000003a0,
0x000003a1,
0x000003b8,
0x000003b9,
0x000003cc,
0x000003cd,
0x000003e0,
0x000003e1,
0x000003a8,
0x000003a9,
0x000003a4,
0x000003a5,
0x000003a6,
0x000003a7,
0x000003a2,
0x000003a3,
0x000003ba,
0x000003bb,
0x000003b4,
0x000003b5,
0x000003b6,
0x000003b7,
0x000003c8,
0x000003c9,
0x000003aa,
0x000003ab,
0x000003c0,
0x000003c1,
0x000003ac,
0x000003ad,
0x000003bc,
0x000003bd,
0x000003ae,
0x000003af,
0x000003be,
0x000003bf,
0x000003c2,
0x000003c3,
0x000003c4,
0x000003c5,
0x000003b0,
0x000003b1,
0x00000309,
0x0000030a,
0x0000030b,
0x0000038d,
0x0000038e,
0x0000038f,
0x00000390,
0x00001900,
/*VMX */

0x00000480,
0x00000481,
0x00000482,
0x00000483,
0x00000484,
0x00000485,
0x00000486,
0x00000487,
0x00000488,
0x00000489,
0x0000048a,
0x0000048b,
0x0000048c,
0x0000048d,
0x0000048e,
0x0000048f,
0x00000490,
0x00000491,
};

void exec_msr_xor(state_t* state_obj, hexa_op* input){
	uint32_t low, high;
	msr_xor* op = (msr_xor*) input;
	UNUSED(state_obj);

	//printf("%s %x\n", __func__, op->msr);
	rdmsr32(msr_kvm_dict[op->msr%(sizeof(msr_kvm_dict)/4)], &low, &high);

	low ^= op->low_xor;
	high ^= op->high_xor;

	wrmsr32(msr_kvm_dict[op->msr%(sizeof(msr_kvm_dict)/4)], low, high);
}

void exec_msr_xor_2(state_t* state_obj, hexa_op* input){
	//uint32_t low, high;
	UNUSED(state_obj);

	msr_xor* op = (msr_xor*) input;

	wrmsr32(msr_kvm_dict[op->msr%(sizeof(msr_kvm_dict)/4)], 0x1000, 0x0);
}

void exec_msr_xor_3(state_t* state_obj, hexa_op* input){
	//uint32_t low, high;
	UNUSED(state_obj);
	msr_xor* op = (msr_xor*) input;

	wrmsr32(msr_kvm_dict[op->msr%(sizeof(msr_kvm_dict)/4)], op->low_xor, op->high_xor);
}

static void exec_mmio_dict_write_data(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_write_dict_8* op = (op_mmio_write_dict_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		//printf("SIZE: %d\n", sizeof(dict)/4);
		offset = dict[op->offset % dict_size()];
		//printf("OFFSET: %x\n", offset);
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_write8(offset, base, data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFF);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_write16(offset, base, data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFFFF);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_write32(offset, base,data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFFFFFFFF);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_write_dict_data_32(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write_data(state_obj, input, 2);
}

void exec_mmio_write_dict_data_16(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write_data(state_obj, input, 1);
}

void exec_mmio_write_dict_data_8(state_t* state_obj, hexa_op* input){
	exec_mmio_dict_write_data(state_obj, input, 0);
}


static void exec_mmio_write_data(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, base;
	op_mmio_write_dict_8* op = (op_mmio_write_dict_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		base = state_obj->mmio_area[slot]->virtual_base;
		
		//printf("SIZE: %d\n", sizeof(dict)/4);
		//offset = dict[op->offset % (sizeof(dict))/4];
		//printf("OFFSET: %x\n", offset);
		//offset %= (state_obj->mmio_area[slot]->size-(1<<size));


		offset = op->offset;
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	//printf("MMIO WRITE8(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_8*)op)->data);
					_mmio_write8(offset, base, data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFF);
					break;
			case 1: //printf("MMIO WRITE16(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_16*)op)->data);
					_mmio_write16(offset, base, data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFFFF);
					break;
			case 2: //printf("MMIO WRITE32(%x, %x, %x)\n\r", offset, base, ((op_mmio_write_32*)op)->data);
					_mmio_write32(offset, base,data_dict[((op_mmio_write_dict_8*)op)->data % data_dict_size()] % 0xFFFFFFFF);
					break;
			default:
					abort();
		}
	}
	asm volatile("" ::: "memory");
}

void exec_mmio_write_data_32(state_t* state_obj, hexa_op* input){
	exec_mmio_write_data(state_obj, input, 2);
}

void exec_mmio_write_data_16(state_t* state_obj, hexa_op* input){
	exec_mmio_write_data(state_obj, input, 1);
}

void exec_mmio_write_data_8(state_t* state_obj, hexa_op* input){
	exec_mmio_write_data(state_obj, input, 0);
}

static void memcpy2(void* dst, void* src, size_t len) {
	//printf("%s %x %x %x\n", __func__, dst, src, len);
	for(size_t i = 0; i < len; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

void __attribute__((optimize("O0"))) exec_x86(state_t* state_obj, hexa_op* input){
	op_x86* op = (op_x86*) input;

	UNUSED(state_obj);
	UNUSED(input);

	set_ret(&&ret_area);
	memcpy2(&&exec_area, op->opcode, 4);
	//printf("go!\n");

	//printf("OPCODE %x\n", ((char*)&&exec_area));

	asm volatile("movl %esp, (0x3000)\n\r"
				"movl $0x3100, %esp\n\r"
				"pusha\n\r"
				"movl %esp, (0x3300)\n\r"
				"movl $0x820400, %eax\n\n"
				"movl $0x820400, %ebx\n\n"
				"movl $0x820400, %ecx\n\n"
				"movl $0x820400, %edx\n\n"
				"movl $0x820400, %edi\n\n"
				"movl $0x820400, %esi\n\n"
				"movl $0x4020, %ebp\n\n"
				"movl $0x4020, %esp\n\n"
				"pushf\n\r"
				"orl $0x000100, (%esp)\n\r"
				"popf\n\r"
				);

	exec_area:
	asm volatile(	" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"

					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"

					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"

					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					/* ret */
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;"
					" nop\n\r;");
	goto ret_area;

ret_area:
	asm volatile("movl (0x3300), %esp\n\r"
				"popa\n\r"
				"movl (0x3000), %esp\n\r");

	printf("EXEC\n");

	return;





}