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
#include "libk.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#include "state.h"
#include "dict.h"
#include "opcodes.h"

void void_handler_decompiler(state_t* state_obj, hexa_op* input){
	(void)(state_obj);
	(void)(input);
	printf("<< DECOMPILER MISSING >>\n\r");
}

static uint32_t decompiler_mmio_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_write_8* op = (op_mmio_write_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		//base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	printf("mmio_write_8   (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_write_8*)op)->data);
					return sizeof(op_mmio_write_8);
			case 1: printf("mmio_write_16  (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_write_16*)op)->data);
					return sizeof(op_mmio_write_16);
			case 2: printf("mmio_write_32  (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_write_32*)op)->data);
					return sizeof(op_mmio_write_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_mmio_write_8);
		case 1: return sizeof(op_mmio_write_16);
		case 2: return sizeof(op_mmio_write_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_mmio_write_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write(state_obj, input, 2);
}

uint32_t decompiler_mmio_write_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write(state_obj, input, 1);
}

uint32_t decompiler_mmio_write_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write(state_obj, input, 0);
}

static uint32_t decompiler_io_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_write_8* op = (op_io_write_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_write_8     (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_16*)op)->data);
					return sizeof(op_io_write_8);
			case 1: printf("io_write_16    (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_16*)op)->data);
					return sizeof(op_io_write_16);
			case 2: printf("io_write_32    (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_16*)op)->data);
					return sizeof(op_io_write_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_io_write_8);
		case 1: return sizeof(op_io_write_16);
		case 2: return sizeof(op_io_write_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_io_write_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_write(state_obj, input, 2);
}

uint32_t decompiler_io_write_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_write(state_obj, input, 1);
}

uint32_t decompiler_io_write_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_write(state_obj, input, 0);
}

uint32_t decompiler_mmio_write_scratch_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, data;
	op_mmio_write_scratch_ptr* op = (op_mmio_write_scratch_ptr*) input;

	if(state_obj->num_alloc_areas && state_obj->num_mmio_areas){
	
		slot = op->slot %state_obj->num_mmio_areas;
		offset = op->offset % (state_obj->mmio_area[slot]->size-4);
		data = ((uint32_t)(state_obj->alloc_areas[op->s_slot % state_obj->num_alloc_areas] + op->s_offset)) % 0x1000;

		printf("mmio_write_32  (0x%x + mmio_area[%d], 0x%x); /* write_scratch_ptr */\n\r", offset, slot, data);

	}

	return sizeof(op_mmio_write_scratch_ptr);
}

uint32_t decompiler_io_write_scratch_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, data;
	op_io_write_scratch_ptr* op = (op_io_write_scratch_ptr*) input;

	if(state_obj->num_alloc_areas && state_obj->num_io_areas){
	
		slot = op->slot %state_obj->num_io_areas;
		offset = op->offset % (state_obj->io_area[slot]->size);//-4);
		data = ((uint32_t)(state_obj->alloc_areas[op->s_slot % state_obj->num_alloc_areas] + op->s_offset)) % 0x1000;

		printf("io_write_32    (0x%x +   io_area[%d], 0x%x); /* write_scratch_ptr */\n\r", offset, slot, data);

	}
	return sizeof(op_io_write_scratch_ptr);
}

static uint32_t decompiler_mmio_memset(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, memset_size;
	op_mmio_memset_8* op = (op_mmio_memset_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;

		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));
		memset_size = (op->size % 0x1000) % (state_obj->mmio_area[slot]->size - offset);


		switch(size){
			case 0:	printf("mmio_memset_8  (0x%x + mmio_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_mmio_memset_8*)op)->data, memset_size);
					return sizeof(op_mmio_memset_8);
			case 1:	printf("mmio_memset_16 (0x%x + mmio_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_mmio_memset_16*)op)->data, memset_size/2);
					return sizeof(op_mmio_memset_8);
			case 2:	printf("mmio_memset_32 (0x%x + mmio_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_mmio_memset_32*)op)->data, memset_size/4);
					return sizeof(op_mmio_memset_8);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_mmio_memset_8);
		case 1: return sizeof(op_mmio_memset_16);
		case 2: return sizeof(op_mmio_memset_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_mmio_memset_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_memset(state_obj, input, 2);
}

uint32_t decompiler_mmio_memset_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_memset(state_obj, input, 1);
}

uint32_t decompiler_mmio_memset_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_memset(state_obj, input, 0);
}

static uint32_t decompiler_io_memset(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot, memset_size;
	op_io_memset_8* op = (op_io_memset_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;

		offset = op->offset % (state_obj->io_area[slot]->size);//-(1<<size));
		memset_size = (op->size % 0xff) % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_memset_8    (0x%x +   io_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_io_memset_8*)op)->data, memset_size);
					return sizeof(op_io_memset_8);
			case 1:	printf("io_memset_16   (0x%x +   io_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_io_memset_16*)op)->data, memset_size/2);
					return sizeof(op_io_memset_8);
			case 2:	printf("io_memset_32   (0x%x +   io_area[%d], 0x%x, 0x%x);\n\r", offset, slot, ((op_io_memset_32*)op)->data, memset_size/4);
					return sizeof(op_io_memset_8);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_io_memset_8);
		case 1: return sizeof(op_io_memset_16);
		case 2: return sizeof(op_io_memset_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_io_memset_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_memset(state_obj, input, 2);
}

uint32_t decompiler_io_memset_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_memset(state_obj, input, 1);
}

uint32_t decompiler_io_memset_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_memset(state_obj, input, 0);
}

static uint32_t decompiler_mmio_xor(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_xor_8* op = (op_mmio_xor_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	printf("mmio_xor_8     (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_xor_8*)op)->data);
					return sizeof(op_mmio_xor_8);
			case 1: printf("mmio_xor_16    (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_xor_16*)op)->data);
					return sizeof(op_mmio_xor_16);
			case 2: printf("mmio_xor_32    (0x%x + mmio_area[%d], 0x%x);\n\r", offset, slot, ((op_mmio_xor_32*)op)->data);
					return sizeof(op_mmio_xor_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_mmio_xor_8);
		case 1: return sizeof(op_mmio_xor_16);
		case 2: return sizeof(op_mmio_xor_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_mmio_xor_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_xor(state_obj, input, 2);
}

uint32_t decompiler_mmio_xor_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_xor(state_obj, input, 1);
}

uint32_t decompiler_mmio_xor_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_xor(state_obj, input, 0);
}


static uint32_t decompiler_io_xor(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_xor_8* op = (op_io_xor_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;

		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_xor_8       (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_xor_8*)op)->data);
					return sizeof(op_io_xor_8);
			case 1: printf("io_xor_16      (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_xor_16*)op)->data);
					return sizeof(op_io_xor_16);
			case 2: printf("io_xor_32      (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_xor_32*)op)->data);
					return sizeof(op_io_xor_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_io_xor_8);
		case 1: return sizeof(op_io_xor_16);
		case 2: return sizeof(op_io_xor_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_io_xor_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_xor(state_obj, input, 2);
}

uint32_t decompiler_io_xor_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_xor(state_obj, input, 1);
}

uint32_t decompiler_io_xor_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_xor(state_obj, input, 0);
}

static uint32_t decompiler_mmio_write_bruteforce(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_bruteforce_8* op = (op_mmio_bruteforce_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	printf("mmio_write_8   (0x%x + mmio_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_mmio_bruteforce_8*)op)->data, op->size);
					return sizeof(op_mmio_bruteforce_8);
			case 1: printf("mmio_write_16  (0x%x + mmio_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_mmio_bruteforce_16*)op)->data, op->size);
					return sizeof(op_mmio_bruteforce_16);
			case 2: printf("mmio_write_32  (0x%x + mmio_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_mmio_bruteforce_32*)op)->data, op->size);
					return sizeof(op_mmio_bruteforce_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_mmio_bruteforce_8);
		case 1: return sizeof(op_mmio_bruteforce_16);
		case 2: return sizeof(op_mmio_bruteforce_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_mmio_write_bruteforce_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write_bruteforce(state_obj, input, 2);	
}

uint32_t decompiler_mmio_write_bruteforce_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write_bruteforce(state_obj, input, 1);	
}

uint32_t decompiler_mmio_write_bruteforce_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_write_bruteforce(state_obj, input, 0);	
}

static uint32_t decompiler_mmio_dict_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_write_dict_8* op = (op_mmio_write_dict_8*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		
		//printf("SIZE: %d\n", sizeof(dict)/4);
		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	printf("mmio_write_8   (0x%x + mmio_area[%d], 0x%x); /* dict offset */\n\r", offset, slot, ((op_mmio_write_8*)op)->data);
					return sizeof(op_mmio_write_dict_8);
			case 1: printf("mmio_write_16  (0x%x + mmio_area[%d], 0x%x); /* dict offset */\n\r", offset, slot, ((op_mmio_write_16*)op)->data);
					return sizeof(op_mmio_write_dict_16);
			case 2: printf("mmio_write_32  (0x%x + mmio_area[%d], 0x%x); /* dict offset */\n\r", offset, slot, ((op_mmio_write_32*)op)->data);
					return sizeof(op_mmio_write_dict_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_mmio_write_dict_8);
		case 1: return sizeof(op_mmio_write_dict_16);
		case 2: return sizeof(op_mmio_write_dict_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_mmio_write_dict_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_dict_write(state_obj, input, 2);
}

uint32_t decompiler_mmio_write_dict_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_dict_write(state_obj, input, 1);
}

uint32_t decompiler_mmio_write_dict_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_dict_write(state_obj, input, 0);
}

uint32_t decompiler_mmio_write_scratch_dict_ptr(state_t* state_obj, hexa_op* input){
	uint32_t offset, slot, data;
	op_mmio_write_scratch_ptr* op = (op_mmio_write_scratch_ptr*) input;

	if(state_obj->num_alloc_areas && state_obj->num_mmio_areas){
	
		slot = op->slot %state_obj->num_mmio_areas;

		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->mmio_area[slot]->size-(1<<2));

		data = ((uint32_t)(state_obj->alloc_areas[op->s_slot % state_obj->num_alloc_areas] + op->s_offset)) % 0x1000;


		printf("mmio_write_32  (0x%x + mmio_area[%d], 0x%x); /* write_scratch_ptr & dict */\n\r", offset, slot, data);

	}
	return sizeof(op_mmio_write_scratch_ptr);
}

static uint32_t decompiler_io_dict_write(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_write_dict_8* op = (op_io_write_dict_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;

		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_write_8     (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_dict_8*)op)->data);
					return sizeof(op_io_write_dict_8);
			case 1: printf("io_write_16    (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_dict_16*)op)->data);
					return sizeof(op_io_write_dict_16);
			case 2: printf("io_write_32    (0x%x +   io_area[%d], 0x%x);\n\r", offset, slot, ((op_io_write_dict_32*)op)->data);
					return sizeof(op_io_write_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_io_write_dict_8);
		case 1: return sizeof(op_io_write_dict_16);
		case 2: return sizeof(op_io_write_dict_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_io_write_dict_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_dict_write(state_obj, input, 2);
}

uint32_t decompiler_io_write_dict_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_dict_write(state_obj, input, 1);
}

uint32_t decompiler_io_write_dict_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_dict_write(state_obj, input, 0);
}

static uint32_t decompiler_mmio_read_dict(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_read_dict* op = (op_mmio_read_dict*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		
		offset = dict[op->offset % dict_size()];
		offset %= (state_obj->mmio_area[slot]->size-(1<<size));
		
		switch(size){
			case 0:	printf("mmio_read_8    (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 1: printf("mmio_read_16   (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 2: printf("mmio_read_32   (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			default:
					abort();
		}
	}
	return sizeof(op_mmio_read_dict);
}

uint32_t decompiler_mmio_read_dict_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read_dict(state_obj, input, 2);
}

uint32_t decompiler_mmio_read_dict_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read_dict(state_obj, input, 1);
}

uint32_t decompiler_mmio_read_dict_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read_dict(state_obj, input, 0);
}

static uint32_t decompiler_io_write_bruteforce(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_bruteforce_8* op = (op_io_bruteforce_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_write_8     (0x%x +   io_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_io_bruteforce_8*)op)->data, op->size);
					return sizeof(op_io_bruteforce_8);
			case 1: printf("io_write_16    (0x%x +   io_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_io_bruteforce_16*)op)->data, op->size);
					return sizeof(op_io_bruteforce_16);
			case 2: printf("io_write_32    (0x%x +   io_area[%d], 0x%x); /* bruteforce X 0x%x */ \n\r", offset, slot, ((op_io_bruteforce_32*)op)->data, op->size);
					return sizeof(op_io_bruteforce_32);
			default:
					abort();
		}
	}
	switch(size){
		case 0:	return sizeof(op_io_bruteforce_8);
		case 1: return sizeof(op_io_bruteforce_16);
		case 2: return sizeof(op_io_bruteforce_32);
		default: abort();
	}
	return 0;
}

uint32_t decompiler_io_write_bruteforce_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_write_bruteforce(state_obj, input, 2);	
}


uint32_t decompiler_io_write_bruteforce_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_write_bruteforce(state_obj, input, 1);	
}

uint32_t decompiler_io_write_bruteforce_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_write_bruteforce(state_obj, input, 0);	
}

static uint32_t decompiler_io_reads(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_reads_8* op = (op_io_reads_8*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_reads_8     (0x%x +   io_area[%d]); /* rep X 0x%x */ \n\r", offset, slot, op->size%0x400);
					break;
			case 1: printf("io_reads_16    (0x%x +   io_area[%d]); /* rep X 0x%x */ \n\r", offset, slot, op->size%0x400);
					break;
			case 2: printf("io_reads_32    (0x%x +   io_area[%d]); /* rep X 0x%x */ \n\r", offset, slot, op->size%0x400);
					break;
			default:
					abort();
		}
	}
	return sizeof(op_io_reads_8);
}

uint32_t decompiler_io_reads_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_reads(state_obj, input, 2);	
}


uint32_t decompiler_io_reads_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_reads(state_obj, input, 1);	
}

uint32_t decompiler_io_reads_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_reads(state_obj, input, 0);	
}


static uint32_t decompiler_io_writes(state_t* state_obj, hexa_op* input, uint8_t size){
	//printf("%s\n", __func__);
	uint32_t offset, slot;
	op_io_writes_8* op = (op_io_writes_8*) input;

	//uint8_t buffer[256];

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_writes_8    (0x%x +   io_area[%d], 0x%x); /* rep X 0x%x */ \n\r", offset, slot, 0x1000, op->size%0x400);
					break;
			case 1: printf("io_writes_16   (0x%x +   io_area[%d], 0x%x); /* rep X 0x%x */ \n\r", offset, slot, 0x1000, op->size%0x400);
					break;
			case 2: printf("io_writes_32   (0x%x +   io_area[%d], 0x%x); /* rep X 0x%x */ \n\r", offset, slot, 0x1000, op->size%0x400);
					break;
			default:
					abort();
		}
	}
	return sizeof(op_io_writes_8);
}

uint32_t decompiler_io_writes_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_writes(state_obj, input, 2);	
}


uint32_t decompiler_io_writes_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_writes(state_obj, input, 1);	
}

uint32_t decompiler_io_writes_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_writes(state_obj, input, 0);	
}


static uint32_t decompiler_mmio_read(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_mmio_read* op = (op_mmio_read*) input;

	if(state_obj->num_mmio_areas){
		slot = op->slot %state_obj->num_mmio_areas;
		
		offset = op->offset % (state_obj->mmio_area[slot]->size-(1<<size));

		switch(size){
			case 0:	printf("mmio_read_8    (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 1: printf("mmio_read_16   (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 2: printf("mmio_read_32   (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			default:
					abort();
		}
	}

	return sizeof(op_mmio_read);
}

uint32_t decompiler_mmio_read_32(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read(state_obj, input, 2);
}

uint32_t decompiler_mmio_read_16(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read(state_obj, input, 1);
}

uint32_t decompiler_mmio_read_8(state_t* state_obj, hexa_op* input){
	return decompiler_mmio_read(state_obj, input, 0);
}


static uint32_t decompiler_io_read(state_t* state_obj, hexa_op* input, uint8_t size){
	uint32_t offset, slot;
	op_io_read* op = (op_io_read*) input;

	if(state_obj->num_io_areas){
		slot = op->slot %state_obj->num_io_areas;
		//base = state_obj->mmio_area[slot]->virtual_base;
		
		offset = op->offset % (state_obj->io_area[slot]->size);

		switch(size){
			case 0:	printf("io_read_8      (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 1: printf("io_read_16     (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			case 2: printf("io_read_32     (0x%x + mmio_area[%d]);\n\r", offset, slot);
					break;
			default:
					abort();
		}
	}

	return sizeof(op_io_read);
}

uint32_t decompiler_io_read_32(state_t* state_obj, hexa_op* input){
	return decompiler_io_read(state_obj, input, 2);
}

uint32_t decompiler_io_read_16(state_t* state_obj, hexa_op* input){
	return decompiler_io_read(state_obj, input, 1);
}

uint32_t decompiler_io_read_8(state_t* state_obj, hexa_op* input){
	return decompiler_io_read(state_obj, input, 0);
}


