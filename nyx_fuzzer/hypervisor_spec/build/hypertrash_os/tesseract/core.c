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
#ifdef HYPERTRASH
#include "tty.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include "state.h"
#include "opcodes.h"
#include "decompiler.h"


typedef struct {
  const char* name; 
  void* exec;
  void* decompiler;
  size_t size;
} handler_t;


handler_t exec_handler[] = {
	
	{"mmio_write_32", exec_mmio_write_32, decompiler_mmio_write_32, sizeof(op_mmio_write_32)},
	{"mmio_write_16", exec_mmio_write_16, decompiler_mmio_write_16, sizeof(op_mmio_write_16)},
	{"mmio_write_8", exec_mmio_write_8, decompiler_mmio_write_8, sizeof(op_mmio_write_8)},
	
	{"io_write_32", exec_io_write_32, decompiler_io_write_32, sizeof(op_io_write_32)},
	{"io_write_16", exec_io_write_16, decompiler_io_write_16, sizeof(op_io_write_16)},
	{"io_write_8", exec_io_write_8, decompiler_io_write_8, sizeof(op_io_write_8)},


	{"mmio_write_scratch_ptr", exec_mmio_write_scratch_ptr, decompiler_mmio_write_scratch_ptr, sizeof(op_mmio_write_scratch_ptr)},
	{"io_write_scratch_ptr", exec_io_write_scratch_ptr, decompiler_io_write_scratch_ptr, sizeof(op_io_write_scratch_ptr)},

	{"mmio_memset_32", exec_mmio_memset_32, decompiler_mmio_memset_32, sizeof(op_mmio_memset_32)},
	{"mmio_memset_16", exec_mmio_memset_16, decompiler_mmio_memset_16, sizeof(op_mmio_memset_16)},
	{"mmio_memset_8", exec_mmio_memset_8, decompiler_mmio_memset_8, sizeof(op_mmio_memset_8)},

	{"io_memset_32", exec_io_memset_32, decompiler_io_memset_32, sizeof(op_io_memset_32)},
	{"io_memset_16", exec_io_memset_16, decompiler_io_memset_16, sizeof(op_io_memset_16)},
	{"io_memset_8", exec_io_memset_8, decompiler_io_memset_8, sizeof(op_io_memset_8)},

	{"mmio_xor_32", exec_mmio_xor_32, decompiler_mmio_xor_32, sizeof(op_mmio_xor_32)},
	{"mmio_xor_16", exec_mmio_xor_16, decompiler_mmio_xor_16, sizeof(op_mmio_xor_16)},
	{"mmio_xor_8", exec_mmio_xor_8, decompiler_mmio_xor_8, sizeof(op_mmio_xor_8)},

	{"io_xor_32", exec_io_xor_32, decompiler_io_xor_32, sizeof(op_io_xor_32)},
	{"io_xor_16", exec_io_xor_16, decompiler_io_xor_16, sizeof(op_io_xor_16)},
	{"io_xor_8", exec_io_xor_8, decompiler_io_xor_8, sizeof(op_io_xor_8)},

	{"mmio_write_bruteforce_32", exec_mmio_write_bruteforce_32, decompiler_mmio_write_bruteforce_32, sizeof(op_mmio_bruteforce_32)},
	{"mmio_write_bruteforce_16", exec_mmio_write_bruteforce_16, decompiler_mmio_write_bruteforce_16, sizeof(op_mmio_bruteforce_16)},
	{"mmio_write_bruteforce_8", exec_mmio_write_bruteforce_8, decompiler_mmio_write_bruteforce_8, sizeof(op_mmio_bruteforce_8)},
 
	{"io_write_bruteforce_32", exec_io_write_bruteforce_32, decompiler_io_write_bruteforce_32, sizeof(op_io_bruteforce_32)},
	{"io_write_bruteforce_16", exec_io_write_bruteforce_16, decompiler_io_write_bruteforce_16, sizeof(op_io_bruteforce_16)},
	{"io_write_bruteforce_8", exec_io_write_bruteforce_8, decompiler_io_write_bruteforce_8, sizeof(op_io_bruteforce_8)},

	{"mmio_write_dict_32", exec_mmio_write_dict_32, decompiler_mmio_write_dict_32, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_dict_16", exec_mmio_write_dict_16, decompiler_mmio_write_dict_16, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_dict_8", exec_mmio_write_dict_8, decompiler_mmio_write_dict_8, sizeof(op_mmio_write_dict_8)},

	{"mmio_write_scratch_dict_ptr", exec_mmio_write_scratch_dict_ptr, decompiler_mmio_write_scratch_dict_ptr, sizeof(op_mmio_write_scratch_dict_ptr)},
	
	{"io_write_dict_32", exec_io_write_dict_32, decompiler_io_write_dict_32, sizeof(op_io_write_dict_32)},
	{"io_write_dict_16", exec_io_write_dict_16, decompiler_io_write_dict_16, sizeof(op_io_write_dict_16)},
	{"io_write_dict_8", exec_io_write_dict_8, decompiler_io_write_dict_8, sizeof(op_io_write_dict_8)},


	{"mmio_read_dict_32", exec_mmio_read_dict_32, decompiler_mmio_read_dict_32, sizeof(op_mmio_read_dict)},
	{"mmio_read_dict_16", exec_mmio_read_dict_16, decompiler_mmio_read_dict_16, sizeof(op_mmio_read_dict)},
	{"mmio_read_dict_8", exec_mmio_read_dict_8, decompiler_mmio_read_dict_8, sizeof(op_mmio_read_dict)},

	{"io_reads_32", exec_io_reads_32, decompiler_io_reads_32, sizeof(op_io_bruteforce_32)},
	{"io_reads_32", exec_io_reads_16, decompiler_io_reads_16, sizeof(op_io_bruteforce_16)},
	{"io_reads_32", exec_io_reads_8, decompiler_io_reads_8, sizeof(op_io_bruteforce_8)},

	{"io_writes_32", exec_io_writes_32, void_handler_decompiler, sizeof(op_io_bruteforce_32)},
	{"io_writes_32", exec_io_writes_16, void_handler_decompiler, sizeof(op_io_bruteforce_16)},
	{"io_writes_32", exec_io_writes_8, void_handler_decompiler, sizeof(op_io_bruteforce_8)},


	{"mmio_read_32", exec_mmio_read_32, decompiler_mmio_read_32, sizeof(op_mmio_read)},
	{"mmio_read_16", exec_mmio_read_16, decompiler_mmio_read_16, sizeof(op_mmio_read)},
	{"mmio_read_8", exec_mmio_read_8, decompiler_mmio_read_8, sizeof(op_mmio_read)},
	
	{"io_read_32", exec_io_read_32, decompiler_io_read_32, sizeof(op_io_read)},
	{"io_read_16", exec_io_read_16, decompiler_io_read_16, sizeof(op_io_read)},
	{"io_read_8", exec_io_read_8, decompiler_io_read_8, sizeof(op_io_read)},
	
	
	//{"vmport_in", exec_vmport_in, void_handler_decompiler, sizeof(vmport_in)},
	//{"vmport_in_scratch_ptr", exec_vmport_in_scratch_ptr, void_handler_decompiler, sizeof(vmport_in_scratch_ptr)},
	
	//{"kvm_hypercall", exec_kvm_hypercall, void_handler_decompiler, sizeof(kvm_hypercall)},

	//{"msr_xor", exec_msr_xor, void_handler_decompiler, sizeof(msr_xor)},
	//{"msr_xor_2", exec_msr_xor_2, void_handler_decompiler, sizeof(msr_xor)},
	//{"msr_xor_3", exec_msr_xor_3, void_handler_decompiler, sizeof(msr_xor)},

	{"mmio_write_dict_data_32", exec_mmio_write_dict_data_32, void_handler_decompiler, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_dict_data_16", exec_mmio_write_dict_data_16, void_handler_decompiler, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_dict_data_8", exec_mmio_write_dict_data_8, void_handler_decompiler, sizeof(op_mmio_write_dict_8)},

	{"mmio_write_data_32", exec_mmio_write_data_32, void_handler_decompiler, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_data_16", exec_mmio_write_data_16, void_handler_decompiler, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_data_8", exec_mmio_write_data_8, void_handler_decompiler, sizeof(op_mmio_write_dict_8)},

	//{"mmio_write_data_8", exec_x86, void_handler_decompiler, sizeof(op_x86)},

};

void run(hexa_op* instructions, uint32_t len, state_t* state_obj){
	hexa_op* input = NULL;
	handler_t* handler;

	reset_state(state_obj);
	
	while(1){
		input = ((void*)instructions) + state_obj->ip;

		handler = &exec_handler[input->op_type%(sizeof(exec_handler)/sizeof(handler_t))];

		if((state_obj->ip + handler->size) >= len){
			return;
		}

#if defined(DISASSEMBLER) || defined(DECOMPILER) || defined(HYBRID)
		if (state_obj->loop_count == 0xFF){
#ifdef DECOMPILER
			//printf("\t");
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
#ifdef DISASSEMBLER
			printf("\t0x%08x\t\t%s\n\r", state_obj->ip, (const char*)handler->name);//, input->arg);
#endif
#ifdef HYBRID
			printf("\t0x%08x\t\t%s\t0x%08x\t\t//", state_obj->ip, (const char*)handler->name, input->arg);
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
		}
#ifdef LOOP
		else{
#ifdef DECOMPILER
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
#ifdef DISASSEMBLER
			printf("0x%02x\t0x%08x\t\t%s\t0x%08x\n", state_obj->loop_count, state_obj->ip, (const char*)handler->name, input->arg);
#endif
#ifdef HYBRID
			printf("0x%02x\t0x%08x\t\t%s\t0x%08x\t\t//", state_obj->loop_count, state_obj->ip, (const char*)handler->name, input->arg);
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
		}	
#endif	
#endif

		((void (*)(state_t* state_obj, hexa_op* input))handler->exec)(state_obj, input); 

		state_obj->ip += handler->size;

		if((state_obj->ip + sizeof(hexa_op)) >= len){
			return;
		}

	}
}
