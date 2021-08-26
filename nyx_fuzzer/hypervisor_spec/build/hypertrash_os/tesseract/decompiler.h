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

void void_handler_decompiler(state_t* state_obj, hexa_op* input);


uint32_t decompiler_mmio_write_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_write_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_write_scratch_ptr(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_scratch_ptr(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_memset_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_memset_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_memset_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_memset_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_memset_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_memset_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_xor_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_xor_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_xor_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_xor_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_xor_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_xor_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_write_bruteforce_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_bruteforce_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_bruteforce_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_write_dict_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_dict_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_write_dict_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_write_scratch_dict_ptr(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_write_dict_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_dict_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_dict_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_read_dict_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_read_dict_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_read_dict_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_write_bruteforce_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_bruteforce_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_write_bruteforce_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_reads_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_reads_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_reads_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_writes_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_writes_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_writes_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_mmio_read_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_read_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_mmio_read_8(state_t* state_obj, hexa_op* input);

uint32_t decompiler_io_read_32(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_read_16(state_t* state_obj, hexa_op* input);
uint32_t decompiler_io_read_8(state_t* state_obj, hexa_op* input);
