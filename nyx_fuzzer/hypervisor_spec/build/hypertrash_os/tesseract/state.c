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
#include "state.h"

#ifdef HYPERTRASH
#include "tty.h"
#include "mem.h"
#include "libk.h"
#define hexa_malloc(x) kmalloc(x)
#define hexa_page_malloc(x) kvmalloc(x)
#define hexa_free(x) kfree(x)

#define STR_PREFIX	" [HEXA] "

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define hexa_malloc(x) malloc(x)
#define hexa_page_malloc(x) malloc(x)
#define hexa_free(x) free(x)
#endif

state_t* new_state(void){
	state_t* state_obj = hexa_malloc(sizeof(state_t));
	memset(state_obj->a, 0x0, sizeof(state_obj->a));

	state_obj->num_alloc_areas = NUM_OF_SCRATCH_AREAS;
	state_obj->alloc_areas = hexa_malloc(sizeof(uintptr_t) * NUM_OF_SCRATCH_AREAS);
	for(uint8_t i = 0; i < NUM_OF_SCRATCH_AREAS; i++){
		state_obj->alloc_areas[i] = hexa_page_malloc(SCRATCH_AREA_SIZE);
#ifdef HYPERTRASH
		printf(STR_PREFIX"Scratch area #%d located at 0x%x\n", i, state_obj->alloc_areas[i]);
#endif
		memset(state_obj->alloc_areas[i], 0x0, SCRATCH_AREA_SIZE);
	}

	state_obj->loop_count = 0xFF;
	state_obj->loop_target = 0;
	state_obj->last_loop_ip = 0;
	state_obj->ip = 0;

	state_obj->num_mmio_areas = 0;
	state_obj->num_io_areas = 0;

	state_obj->mmio_area = NULL;
	state_obj->io_area = NULL;	

	return state_obj;
}

void print_state(state_t* state){
	printf("============== CONFIGURATION ==============\n\r");
	for(uint16_t i = 0; i < state->num_mmio_areas; i++){
		printf("mmio_area[%d] =\t{\n\r\t\t  base = %0x;\n\r\t\t  size = 0x%x;\n\r\t\t  desc = \"%s\";\n\r\t\t};\n\r", i, state->mmio_area[i]->base, state->mmio_area[i]->size, state->mmio_area[i]->desc);
	}
	for(uint16_t i = 0; i < state->num_io_areas; i++){
		printf("io_area[%d] =\t{\n\r\t\t  base = 0x%x;\n\r\t\t  size = 0x%x;\n\r\t\t  desc = \"%s\";\n\r\t\t};\n\r", i, state->io_area[i]->base, state->io_area[i]->size, state->io_area[i]->desc);
	}
	printf("===========================================\n\r");

	#ifndef LOOP
		printf("WARNING: loop counts are concealed from decompiler output (use hexas_32_decompiler_loop instead)!\n");
	#endif 

	printf("\n\r============ DECOMPILER OUTPUT ============\n\r");
}

state_t* load_state(void* config_data, uint32_t config_len){
	state_t* self = new_state();

	if(config_len < sizeof(config_t)){
		printf("%s error: illegal size (%x)...\n", __func__, config_len);
		abort();
	}

	config_t* config = (config_t*) config_data;

	if(config->magic != MAGIC_NUMBER){
		printf("%s error: wrong magic number (%x != %x)...\n", __func__, config->magic, MAGIC_NUMBER);
		abort();
	}

	if(config->num_alloc_areas != self->num_alloc_areas){
		printf("%s error: wrong number of alloc areas (%x != %x)...\n", __func__, config->num_alloc_areas, self->num_alloc_areas);
		abort();	
	}

	uint32_t expected_file_size = (config->num_mmio_areas*sizeof(area_t_export) + config->num_io_areas*sizeof(area_t_export) + sizeof(config_t));
	if(expected_file_size != config_len){
		printf("%s error: expected config size mismatch (%x != %x)...\n", __func__, expected_file_size, config_len);
		abort();
	}

	self->num_mmio_areas = config->num_mmio_areas;
	self->num_io_areas = config->num_io_areas;
	self->mmio_area = (area_t**)hexa_malloc(sizeof(area_t*) * (size_t)config->num_mmio_areas);
	self->io_area = (area_t**)hexa_malloc(sizeof(area_t*) * (size_t)config->num_io_areas);

	area_t_export* tmp = NULL; 

	for(uint16_t i = 0; i < config->num_mmio_areas; i++){
		tmp = (area_t_export*)(config_data + (sizeof(config_t)) + (sizeof(area_t_export) * i));
		self->mmio_area[i] = (area_t*)hexa_malloc(sizeof(area_t));
		memset(self->mmio_area[i], 0x0, sizeof(area_t));
		self->mmio_area[i]->base = tmp->base;
		self->mmio_area[i]->size = tmp->size;
		memcpy(self->mmio_area[i]->desc, tmp->desc, 255);
		self->mmio_area[i]->desc[255] = 0;
	}

	for(uint16_t i = 0; i < config->num_io_areas; i++){
		tmp = (area_t_export*)(config_data + (sizeof(config_t)) + (sizeof(area_t_export) * config->num_mmio_areas) + (sizeof(area_t_export) * i));

		self->io_area[i] = (area_t*)hexa_malloc(sizeof(area_t));
		memset(self->io_area[i], 0x0, sizeof(area_t));
		self->io_area[i]->base = tmp->base;;
		self->io_area[i]->size = tmp->size;
		memcpy(self->io_area[i]->desc, tmp->desc, 255);
		self->io_area[i]->desc[255] = 0;
	}

	printf("============== CONFIGURATION ==============\n");
	for(uint16_t i = 0; i < config->num_mmio_areas; i++){
		printf("mmio_area[%d] =\t{\n\t\t  base = %0x;\n\t\t  size = 0x%x;\n\t\t  desc = \"%s\";\n\t\t};\n", i, self->mmio_area[i]->base, self->mmio_area[i]->size, self->mmio_area[i]->desc);
	}
	for(uint16_t i = 0; i < config->num_io_areas; i++){
		printf("io_area[%d] =\t{\n\t\t  base = 0x%x;\n\t\t  size = 0x%x;\n\t\t  desc = \"%s\";\n\t\t};\n", i, self->io_area[i]->base, self->io_area[i]->size, self->io_area[i]->desc);
	}
	printf("===========================================\n");

	#ifndef LOOP
		printf("WARNING: loop counts are concealed from decompiler output (use hexas_32_decompiler_loop instead)!\n");
	#endif 

	printf("\n============ DECOMPILER OUTPUT ============\n");

	return self;
}


/* serialize the configuration data */
void save_state(state_t* state_data, void** saved_data, uint32_t* saved_len){

	*saved_len = sizeof(config_t) + (sizeof(area_t_export)*(state_data->num_mmio_areas)) + (sizeof(area_t_export)*(state_data->num_io_areas));
	*saved_data = hexa_page_malloc(*saved_len);
	memset(*saved_data, 0x0, *saved_len);

	config_t* config =			((config_t*)(*saved_data));
	area_t_export* mmio_areas = ((area_t_export*)((void*)config + sizeof(config_t)));
	area_t_export* io_areas =	(area_t_export*)(((void*)config + sizeof(config_t) + (sizeof(area_t_export) * state_data->num_mmio_areas)));

	config->magic = MAGIC_NUMBER;
	config->num_mmio_areas = state_data->num_mmio_areas;
	config->num_io_areas = state_data->num_io_areas;
	config->num_alloc_areas = state_data->num_alloc_areas;

	for(uint16_t i = 0; i < state_data->num_mmio_areas; i++){
		mmio_areas[i].base = state_data->mmio_area[i]->base;
		mmio_areas[i].size = state_data->mmio_area[i]->size;
		mmio_areas[i].virtual_base = state_data->mmio_area[i]->virtual_base;
		memcpy(mmio_areas[i].desc, state_data->mmio_area[i]->desc, 255);
	}

	for(uint16_t i = 0; i < state_data->num_io_areas; i++){
		io_areas[i].base = state_data->io_area[i]->base;
		io_areas[i].size = state_data->io_area[i]->size;
		io_areas[i].virtual_base = 0;
		memcpy(io_areas[i].desc, state_data->io_area[i]->desc, 255);
	}
}

void destroy_state(state_t* state_obj){
	for(uint16_t i = 0; i < state_obj->num_mmio_areas; i++){
		hexa_free(state_obj->mmio_area[i]);
	}

	for(uint16_t i = 0; i < state_obj->num_io_areas; i++){
		hexa_free(state_obj->io_area[i]);
	}

	if(state_obj->mmio_area){
		hexa_free(state_obj->mmio_area);
	}
	if(state_obj->io_area){
		hexa_free(state_obj->io_area);
	}

	for(uint8_t i = 0; i < NUM_OF_SCRATCH_AREAS; i++){
		hexa_free(state_obj->alloc_areas[i]);
	}
	hexa_free(state_obj->alloc_areas);
	hexa_free(state_obj);
}

void reset_state(state_t* state_obj){
	memset(state_obj->a, 0x0, sizeof(state_obj->a));

	state_obj->loop_count = 0xFF;
	state_obj->loop_target = 0;
	state_obj->last_loop_ip = 0;
	state_obj->ip = 0;
}
