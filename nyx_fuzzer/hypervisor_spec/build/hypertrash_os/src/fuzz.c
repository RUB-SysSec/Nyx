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


#include "kafl_user.h"
#include "tty.h"
#include "state.h"
#include "core.h"
#include "libk.h"
#include "mem.h"
#include "config.h"
#include "fuzz.h"
#include "hypercube_opcodes.h"
#include "../../interpreter.h"

//#define DEBUG

#define STR_PREFIX	" [FUZZ] "

#ifdef PAYLOAD
extern uint8_t _binary_misc_crash_hexa_start;
extern uint8_t _binary_misc_crash_hexa_end;

#endif

void init_hprintf(void){
	alloc_nested_hprintf_buffer((void*)kvmalloc(1024));
}

void exec_hprintf_str(char* buffer) {
	nested_hprintf_str(buffer);
}

fuzzer_state_t* new_fuzzer(void){
	fuzzer_state_t* self = NULL;

	memset(0, 0, 0x1000);

	self = kmalloc(sizeof(fuzzer_state_t));
	self->hexa_state = new_state();

	self->hexa_state->mmio_area = kmalloc(sizeof(area_t*) * 256);
	self->hexa_state->io_area = kmalloc(sizeof(area_t*) * 256);


	self->address_page = (uintptr_t)kvmalloc(0x1000);
	memset((void*)self->address_page, 0x0, 0x1000);


	self->hexa_state->num_alloc_areas = 2;
	self->hexa_state->alloc_areas = (uint8_t**)kmalloc(sizeof(uint8_t*) * 2);
	self->hexa_state->alloc_areas[0] = kvmalloc(0x1000);
	memset(self->hexa_state->alloc_areas[0], 0xff, 0x1000);
	self->hexa_state->alloc_areas[1] = kvmalloc(0x1000);

	printf(STR_PREFIX"Allocating 64kB memory for fuzzer payload buffer\n\r");
	self->payload_buffer = (uintptr_t)kvmalloc( (64 * 1024)); // Alloc 64kB
	printf(STR_PREFIX"payload_buffer = 0x%x-0x%x\n\r", self->payload_buffer, self->payload_buffer+(64*1024));
	memset((void*)self->payload_buffer, 0xff, 64*1024);
	printf(STR_PREFIX"Payload buffer prepared!\n\r");

	uint32_t address = self->payload_buffer;

		
	for (uint32_t i = 0; i < (64/4); i++){
		*((uint32_t*)(self->address_page + (0x8 * i))) = address;
		printf(STR_PREFIX"REQ HYPERCALL (%x:%x) [PAYLOAD BUFFER SETUP]\n\r", i, address);
		address += 4096;
	}

	printf("Ready!\n\r");

	return self;
}

void destroy_fuzzer(fuzzer_state_t* self){
	UNUSED(self);
	/* ... */
	return;
}

int i = 0;
void register_area(fuzzer_state_t* self, uintptr_t base_address, uint32_t size, bool mmio, char* description){

#ifdef FILTER
	if(strcmp(description, FILTER))
		return;
#endif

	printf(STR_PREFIX"%s - base:0x%x size:0x%x mmio:0x%d -> %s\n\r", __func__, base_address, size, mmio, description);

	if(!mmio){
		//	printf(STR_PREFIX"%s - base:0x%x size:0x%x mmio:0x%d -> %s\n\r", __func__, base_address, size, mmio, description);
		self->hexa_state->io_area[self->hexa_state->num_io_areas] = kvmalloc(sizeof(area_t));
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->base = base_address;
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->size = size;
		memcpy(self->hexa_state->io_area[self->hexa_state->num_io_areas]->desc, description, strlen(description));
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->virtual_base = 0;
		self->hexa_state->num_io_areas++;		
	}
	
	else{
		//printf(STR_PREFIX"%s - base:0x%x size:0x%x mmio:0x%d -> %s\n\r", __func__, base_address, size, mmio, description);
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas] = kvmalloc(sizeof(area_t));
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->base = base_address;
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->size = size;
		memcpy(self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->desc, description, strlen(description));
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->virtual_base = (uintptr_t)virtual_map(base_address, size);
		self->hexa_state->num_mmio_areas++;
	}

}

static __attribute__((unused)) void provide_addresses(fuzzer_state_t* self){
	printf("%s\n\r", __func__);
	kAFL_hypercall(HYPERCALL_KAFL_NESTED_PREPARE, self->address_page | 16);
}

static __attribute__((unused)) void provide_configuration(fuzzer_state_t* self){

	nested_hprintf("%s!\n", __func__);
	printf("%s\n\r", __func__);

	void* saved_data = NULL;
	uint32_t saved_len = 0;

	save_state(self->hexa_state, &saved_data, &saved_len);

	printf("%s PRE HYPERCALL\n\r", __func__);

	assert2(saved_len < 0x10000-sizeof(uint32_t), "saved_len < 0x10000-sizeof(uint32_t)");

	uint8_t* config_buffer = (uint8_t*)self->payload_buffer;
	*((uint32_t*)config_buffer) = saved_len;
	memcpy(((void*)config_buffer)+sizeof(uint32_t), saved_data, saved_len);

	printf("%s POST HYPERCALL\n\r", __func__);

	kAFL_hypercall(HYPERCALL_KAFL_NESTED_CONFIG, 0);
	printf("LEN: %x\n", saved_len);

	printf("%s FIN\n\r", __func__);
}

uint8_t* prepare_fuzzing(volatile fuzzer_state_t* self){
#ifndef PAYLOAD

	enable_printf();
	disable_printf();
	print_state(self->hexa_state);

	provide_addresses((fuzzer_state_t*)self);
	provide_configuration((fuzzer_state_t*)self);

	return (uint8_t*)self->payload_buffer;
#else
	UNUSED(self);
	return (uint8_t*)&_binary_misc_crash_hexa_start;
#endif
}

void print_xhci_cap(uintptr_t base){
	printf("CAPLENGTH: -> 0x%x\n", *((volatile uint32_t*)(base)));
	printf("HCSPARAMS1: -> 0x%x\n", *((volatile uint32_t*)(base+0x4)));
	printf("HCSPARAMS2: -> 0x%x\n", *((volatile uint32_t*)(base+0x8)));
	printf("HCSPARAMS3: -> 0x%x\n", *((volatile uint32_t*)(base+0xc)));
	printf("HCCPARAMS: -> 0x%x\n", *((volatile uint32_t*)(base+0x10)));
	printf("DBOFF: -> 0x%x\n", *((volatile uint32_t*)(base+0x14)));
	printf("RTSOFF: -> 0x%x\n", *((volatile uint32_t*)(base+0x18)));
	printf("HCCPRAMS2: -> 0x%x\n", *((volatile uint32_t*)(base+0x1c)));
}

void start_fuzzing(uint8_t* payload_buffer, size_t payload_size){

	interpreter_t* vm = new_interpreter();

	hypertrash_context_t context;
	memset(&context, 0x0, sizeof(hypertrash_context_t));

	uintptr_t dma_buffer_virt_addr = 0;
	uintptr_t dma_buffer_phys_addr = 0;
	size_t dma_buffer_size = 0x1000*32; 
	dma_buffer_virt_addr = (uintptr_t)kvmalloc_p(dma_buffer_size, &dma_buffer_phys_addr);
	memset((void*)dma_buffer_virt_addr, 0x0, dma_buffer_size);

	context.dma_buffer_size = 0x1000*16;
	context.dma_buffer_virt_addr = dma_buffer_virt_addr;
	context.dma_buffer_phys_addr = dma_buffer_phys_addr;

	vm->user_data = &context; 

#ifndef PAYLOAD
	while(1){
		kAFL_hypercall(HYPERCALL_KAFL_NESTED_ACQUIRE, 0);
#endif

    	uint64_t* offsets = (uint64_t*)payload_buffer;
			//printf("checksum: %x, %x\n", (uint32_t)offsets[0], (uint32_t)INTERPRETER_CHECKSUM);
#ifndef PAYLOAD
    	assert2((uint32_t)offsets[0] == (uint32_t)INTERPRETER_CHECKSUM, "checksum");
#endif
    	assert2(offsets[1] < 0xffffff, "offset1");
    	assert2(offsets[2] < 0xffffff, "offset2");
    	assert2(offsets[3] < 0xffffff, "offset3");
    	assert2(offsets[4] < 0xffffff, "offset4");
    	uint64_t graph_size = offsets[1];
    	uint64_t data_size = offsets[2];
    	//printf("graph_size: %d\n", (uint32_t)graph_size);
    	//printf("data_size: %d\n", (uint32_t)data_size);
    	//printf("graph_offset: %d\n", (uint32_t)offsets[3]);
    	//printf("data_offset: %d\n", (uint32_t)offsets[4]);
    	uint16_t* graph_ptr = (uint16_t*)(payload_buffer+offsets[3]);
    	uint8_t* data_ptr = (uint8_t*)(payload_buffer+offsets[4]);
    	assert2(offsets[3]+graph_size*sizeof(uint16_t) <= payload_size, "graph_size");
    	assert2(offsets[4]+data_size <= payload_size, "data_size");

    	init_interpreter(vm, graph_ptr, graph_size, data_ptr, data_size);
    	interpreter_user_init(vm);
    	interpreter_run(vm);
    	interpreter_user_shutdown(vm);
    	//printf("testcase finished\n");

#ifndef PAYLOAD
		kAFL_hypercall(HYPERCALL_KAFL_NESTED_RELEASE, 0);

	}
#endif

	return;
}
