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

#ifndef MSR_H
#define MSR_H

#include <stdbool.h>
#include <stdint.h>

#define IA32_APIC_BASE		0x0000001b
#define IA32_EFER 			0xc0000080


void rdmsr32(uint32_t msr, uint32_t *lo, uint32_t *hi);
void wrmsr32(uint32_t msr, uint32_t lo, uint32_t hi);

uint64_t rdmsr64(uint32_t msr);
void wrmsr64(uint32_t msr, uint64_t value);

#endif

