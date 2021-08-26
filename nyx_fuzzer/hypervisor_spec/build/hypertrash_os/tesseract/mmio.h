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

#ifdef HYPERTRASH
#include "tty.h"
#endif


#ifdef HYPERTRASH
static inline uint8_t _mmio_read8(uint32_t __base, uint32_t __offset){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
       return *(volatile uint8_t *)((uintptr_t)(__base + __offset));
}

static inline uint16_t _mmio_read16(uint32_t __base, uint32_t __offset){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
       return *(volatile uint16_t *)((uintptr_t)(__base + __offset));
}

static inline uint32_t _mmio_read32(uint32_t __base, uint32_t __offset){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
       return *(volatile uint32_t *)((uintptr_t)(__base + __offset));
}

static inline uint64_t _mmio_read64(uint32_t __base, uint32_t __offset){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
       return *(volatile uint64_t *)((uintptr_t)(__base + __offset));
}

static inline void _mmio_write8(uint32_t __base, uint32_t __offset, uint8_t data){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	*(volatile uint8_t *)((uintptr_t)(__base + __offset)) = data;
}

static inline void _mmio_write16(uint32_t __base, uint32_t __offset, uint16_t data){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	*(volatile uint16_t *)((uintptr_t)(__base + __offset)) = data;
}

static inline void _mmio_write32(uint32_t __base, uint32_t __offset, uint32_t data){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	*(volatile uint32_t *)((uintptr_t)(__base + __offset)) = data;
}

static inline void _mmio_write64(uint32_t __base, uint32_t __offset, uint64_t data){
#ifdef INTERPRETER_BENCHMARK
	return 0;
#endif
	*(volatile uint64_t *)((uintptr_t)(__base + __offset)) = data;
}


#else
#define _mmio_read8(__base, __offset) 0
#define _mmio_read16(__base, __offset) 0
#define _mmio_read32(__base, __offset) 0
#define _mmio_read64(__base, __offset) 0

#define _mmio_write8(__base, __offset, __data) {}
#define _mmio_write16(__base, __offset, __data) {}
#define _mmio_write32(__base, __offset, __data) {}
#define _mmio_write64(__base, __offset, __data) {}

#endif
