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

#include <stdint.h>

#define MAX_LOOP_COUNT	0x8
#define MAX_LOOP_STEPS	0x8

#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)

#define AREA_DESC_LEN			256
#define MAGIC_NUMBER			0x41584548U
#define NUM_OF_SCRATCH_AREAS	1
#define SCRATCH_AREA_SIZE		0x2000

typedef struct {
	uint8_t op_type;
	//uint8_t op_code;
	uint32_t arg;
} __attribute__((packed)) hexa_op;

typedef struct {
	uint32_t base;
	uint32_t size;
	uint32_t virtual_base;
	char desc[AREA_DESC_LEN];
}area_t_export;

typedef struct {
	uint32_t base;
	uint32_t size;
	uint32_t virtual_base;
	char desc[AREA_DESC_LEN];
}area_t;

typedef struct {
	uint32_t a[10];
	uint8_t num_mmio_areas;
	uint8_t num_io_areas;
	uint8_t num_alloc_areas;
	uint8_t** alloc_areas; 
	area_t** mmio_area; 
	area_t** io_area;

	uint8_t loop_count; 
	uint32_t ip;
	uint32_t loop_target;
	uint32_t last_loop_ip;
}state_t;

typedef struct {
	uint32_t magic;
	uint8_t num_mmio_areas;
	uint8_t num_io_areas;
	uint8_t num_alloc_areas;
	uint8_t padding;
}config_t;

state_t* new_state(void);
void print_state(state_t* state);
state_t* load_state(void* config_data, uint32_t config_len);
void save_state(state_t* state_data, void** saved_data, uint32_t* saved_len);
void destroy_state(state_t* state_obj);
void reset_state(state_t* state_obj);
