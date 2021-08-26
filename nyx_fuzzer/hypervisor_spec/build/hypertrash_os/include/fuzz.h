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
//
#include <stddef.h>
#include "state.h"

//#include <stdbool.h>
//
typedef struct {
	state_t* hexa_state;
	uintptr_t address_page;
	volatile uintptr_t payload_buffer; 
}fuzzer_state_t;

fuzzer_state_t* new_fuzzer(void);
void destroy_fuzzer(fuzzer_state_t* self);
void register_area(fuzzer_state_t* self, uintptr_t base_address, uint32_t size, bool mmio, char* description);
//void start_fuzzing(volatile fuzzer_state_t* self);

void init_hprintf(void);
uint8_t* prepare_fuzzing(volatile fuzzer_state_t* self);
void start_fuzzing(uint8_t* payload_buffer, size_t payload_size);
void exec_hprintf_str(char* buffer);