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

#include "msr.h"

const uint32_t CPUID_FLAG_MSR = 1 << 5;
 
void rdmsr32(uint32_t msr, uint32_t *lo, uint32_t *hi){
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
 
void wrmsr32(uint32_t msr, uint32_t lo, uint32_t hi){
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
 
uint64_t rdmsr64(uint32_t msr){
	uint32_t lo, hi;
	uint64_t result;
	asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	result = hi;
	result = result << 32;
	result = result + lo;
	return result;
}
 
void wrmsr64(uint32_t msr, uint64_t value){
   asm volatile("wrmsr" : : "a"((uint32_t)(value)), "d"((uint32_t)(value>>32)), "c"(msr));
}