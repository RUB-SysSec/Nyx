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

#pragma once 

#include "state.h"

void base_exec_handler(state_t* state_obj, hexa_op* input);
void void_handler(state_t* state_obj, hexa_op* input);

enum { 
	magic_op_mmio_write_32 = 0,
	magic_op_mmio_write_16,
	magic_op_mmio_write_8,

	magic_op_io_write_32,
	magic_op_io_write_16,
	magic_op_io_write_8,

	magic_op_mmio_write_scratch_ptr,
	magic_op_io_write_scratch_ptr,

	magic_op_mmio_memset_32,
	magic_op_mmio_memset_16,
	magic_op_mmio_memset_8,

	magic_op_io_memset_32,
	magic_op_io_memset_16,
	magic_op_io_memset_8,

	magic_op_mmio_xor_32,
	magic_op_mmio_xor_16,
	magic_op_mmio_xor_8,

	magic_op_io_xor_32,
	magic_op_io_xor_16,
	magic_op_io_xor_8,

	magic_op_mmio_bruteforce_32,
	magic_op_mmio_bruteforce_16,
	magic_op_mmio_bruteforce_8,

	magic_op_mmio_write_dict_32,
	magic_op_mmio_write_dict_16,
	magic_op_mmio_write_dict_8,

	magic_op_mmio_write_scratch_dict_ptr,

	magic_op_io_write_dict_32,
	magic_op_io_write_dict_16,
	magic_op_io_write_dict_8,

	magic_op_mmio_read_dict_32,
	magic_op_mmio_read_dict_16,
	magic_op_mmio_read_dict_8,

	magic_op_io_bruteforce_32,
	magic_op_io_bruteforce_16,
	magic_op_io_bruteforce_8,

};

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_mmio_write_32;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_mmio_write_16;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_mmio_write_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_io_write_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_io_write_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_io_write_8;


typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t s_slot;
	uint16_t s_offset;
} __attribute__((packed)) op_mmio_write_scratch_ptr;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t s_slot;
	uint16_t s_offset;
} __attribute__((packed)) op_io_write_scratch_ptr;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint32_t data;
} __attribute__((packed)) op_mmio_memset_32;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint16_t data;
} __attribute__((packed)) op_mmio_memset_16;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint8_t data;
} __attribute__((packed)) op_mmio_memset_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint32_t data;
} __attribute__((packed)) op_io_memset_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint16_t data;
} __attribute__((packed)) op_io_memset_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint8_t data;
} __attribute__((packed)) op_io_memset_8;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_mmio_xor_32;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_mmio_xor_16;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_mmio_xor_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_io_xor_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_io_xor_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_io_xor_8;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint32_t data;
} __attribute__((packed)) op_mmio_bruteforce_32;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint16_t data;
} __attribute__((packed)) op_mmio_bruteforce_16;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
	uint8_t size;
	uint8_t data;
} __attribute__((packed)) op_mmio_bruteforce_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_mmio_write_dict_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_mmio_write_dict_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_mmio_write_dict_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t s_slot;
	uint16_t s_offset;
} __attribute__((packed)) op_mmio_write_scratch_dict_ptr;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint32_t data;
} __attribute__((packed)) op_io_write_dict_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint16_t data;
} __attribute__((packed)) op_io_write_dict_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t data;
} __attribute__((packed)) op_io_write_dict_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
} __attribute__((packed)) op_mmio_read_dict;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint32_t data;
} __attribute__((packed)) op_io_bruteforce_32;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint16_t data;
} __attribute__((packed)) op_io_bruteforce_16;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
	uint8_t data;
} __attribute__((packed)) op_io_bruteforce_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
} __attribute__((packed)) op_io_reads_8;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
	uint8_t size;
} __attribute__((packed)) op_io_writes_8;

typedef struct {
	uint8_t op_type;
	uint32_t offset;
	uint8_t slot;
} __attribute__((packed)) op_mmio_read;

typedef struct {
	uint8_t op_type;
	uint16_t offset;
	uint8_t slot;
} __attribute__((packed)) op_io_read;

typedef struct {
	uint8_t op_type;
	uint32_t ecx;
	uint32_t ebx;
} __attribute__((packed)) vmport_in;

typedef struct {
	uint8_t op_type;
	uint32_t ecx;
} __attribute__((packed)) vmport_in_scratch_ptr;

typedef struct {
	uint8_t op_type;
	uint8_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
} __attribute__((packed)) kvm_hypercall;

typedef struct {
	uint8_t op_type;
	uint16_t msr;
	uint32_t low_xor;
	uint32_t high_xor;

} __attribute__((packed)) msr_xor;

typedef struct {
	uint8_t op_type;
	uint8_t opcode[16];
} __attribute__((packed)) op_x86;

/*

	mmio_write_32 		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 4B DATA]
	mmio_write_16 		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 2B DATA]
	mmio_write_8  		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B DATA]

	io_write_32 		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 4B DATA]
	io_write_16 		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 2B DATA]
	io_write_8  		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 1B DATA]

	mmio_write_addr 	[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B SSLOT + 2B SOFFSET]
	io_write_addr 	[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B SSLOT + 2B SOFFSET]

	mmio_memset_32 		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B SIZE + 4B DATA]
	mmio_memset_16		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B SIZE + 2B DATA]
	mmio_memset_8  		[ 1B MAGIC + 4B OFFSET + 1B SLOT  + 1B SIZE + 1B DATA]

	io_memset_32  		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 1B SIZE + 4B DATA]
	io_memset_16  		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 1B SIZE + 2B DATA]
	io_memset_8   		[ 1B MAGIC + 2B OFFSET + 1B SLOT  + 1B SIZE + 1B DATA]

*/

void exec_mmio_write_32(state_t* state_obj, hexa_op* input);
void exec_mmio_write_16(state_t* state_obj, hexa_op* input);
void exec_mmio_write_8(state_t* state_obj, hexa_op* input);

void exec_io_write_32(state_t* state_obj, hexa_op* input);
void exec_io_write_16(state_t* state_obj, hexa_op* input);
void exec_io_write_8(state_t* state_obj, hexa_op* input);

void exec_mmio_write_scratch_ptr(state_t* state_obj, hexa_op* input);
void exec_io_write_scratch_ptr(state_t* state_obj, hexa_op* input);

void exec_mmio_memset_32(state_t* state_obj, hexa_op* input);
void exec_mmio_memset_16(state_t* state_obj, hexa_op* input);
void exec_mmio_memset_8(state_t* state_obj, hexa_op* input);

void exec_io_memset_32(state_t* state_obj, hexa_op* input);
void exec_io_memset_16(state_t* state_obj, hexa_op* input);
void exec_io_memset_8(state_t* state_obj, hexa_op* input);

void exec_mmio_xor_32(state_t* state_obj, hexa_op* input);
void exec_mmio_xor_16(state_t* state_obj, hexa_op* input);
void exec_mmio_xor_8(state_t* state_obj, hexa_op* input);

void exec_io_xor_32(state_t* state_obj, hexa_op* input);
void exec_io_xor_16(state_t* state_obj, hexa_op* input);
void exec_io_xor_8(state_t* state_obj, hexa_op* input);


void exec_mmio_write_bruteforce_32(state_t* state_obj, hexa_op* input);
void exec_mmio_write_bruteforce_16(state_t* state_obj, hexa_op* input);
void exec_mmio_write_bruteforce_8(state_t* state_obj, hexa_op* input);


void exec_mmio_write_dict_32(state_t* state_obj, hexa_op* input);
void exec_mmio_write_dict_16(state_t* state_obj, hexa_op* input);
void exec_mmio_write_dict_8(state_t* state_obj, hexa_op* input);

void exec_mmio_write_scratch_dict_ptr(state_t* state_obj, hexa_op* input);

void exec_io_write_dict_32(state_t* state_obj, hexa_op* input);
void exec_io_write_dict_16(state_t* state_obj, hexa_op* input);
void exec_io_write_dict_8(state_t* state_obj, hexa_op* input);


void exec_mmio_read_dict_32(state_t* state_obj, hexa_op* input);
void exec_mmio_read_dict_16(state_t* state_obj, hexa_op* input);
void exec_mmio_read_dict_8(state_t* state_obj, hexa_op* input);


void exec_io_write_bruteforce_32(state_t* state_obj, hexa_op* input);
void exec_io_write_bruteforce_16(state_t* state_obj, hexa_op* input);
void exec_io_write_bruteforce_8(state_t* state_obj, hexa_op* input);


void exec_io_reads_32(state_t* state_obj, hexa_op* input);
void exec_io_reads_16(state_t* state_obj, hexa_op* input);
void exec_io_reads_8(state_t* state_obj, hexa_op* input);

void exec_io_writes_32(state_t* state_obj, hexa_op* input);
void exec_io_writes_16(state_t* state_obj, hexa_op* input);
void exec_io_writes_8(state_t* state_obj, hexa_op* input);

void exec_mmio_read_32(state_t* state_obj, hexa_op* input);
void exec_mmio_read_16(state_t* state_obj, hexa_op* input);
void exec_mmio_read_8(state_t* state_obj, hexa_op* input);

void exec_io_read_32(state_t* state_obj, hexa_op* input);
void exec_io_read_16(state_t* state_obj, hexa_op* input);
void exec_io_read_8(state_t* state_obj, hexa_op* input);

void exec_vmport_in(state_t* state_obj, hexa_op* input);
void exec_vmport_in_scratch_ptr(state_t* state_obj, hexa_op* input);

void exec_kvm_hypercall(state_t* state_obj, hexa_op* input);

void exec_msr_xor(state_t* state_obj, hexa_op* input);
void exec_msr_xor_2(state_t* state_obj, hexa_op* input);
void exec_msr_xor_3(state_t* state_obj, hexa_op* input);


void exec_mmio_write_dict_data_32(state_t* state_obj, hexa_op* input);
void exec_mmio_write_dict_data_16(state_t* state_obj, hexa_op* input);
void exec_mmio_write_dict_data_8(state_t* state_obj, hexa_op* input);


void exec_mmio_write_data_32(state_t* state_obj, hexa_op* input);
void exec_mmio_write_data_16(state_t* state_obj, hexa_op* input);
void exec_mmio_write_data_8(state_t* state_obj, hexa_op* input);

void exec_x86(state_t* state_obj, hexa_op* input);

