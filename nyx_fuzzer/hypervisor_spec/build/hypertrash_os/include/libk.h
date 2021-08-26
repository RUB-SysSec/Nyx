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

#include "system.h"
#include "cpuid.h"

//#define MEMCPY_DEBUG

void microdelay(uint32_t us);

typedef __builtin_va_list va_list;
#ifndef va_start
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_copy(dest, src) __builtin_va_copy(dest,src)
#endif

size_t vasprintf(char * buf, const char *fmt, va_list args);
int sprintf(char * buf, const char *fmt, ...);

size_t strlen(const char* str);
void memcpy(void* dst, void* src, size_t len);
void* memset(void* dst, int c, size_t len);

char* dump_cpuid();
int raise_gpf();
void abort(void);
void assert(bool cond);
void assert2(bool cond, const char* reason);

void hexdump (char *desc, void *addr, int len);

int strcmp(const char *str1, const char *str2);
size_t vsnprintf(char * buf, size_t n, const char *fmt, va_list args);

char* strncpy(char* dst, const char* src, size_t num);

/*
static inline uint64_t rdtsc()
{
    uint64_t ret;
    asm volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}
*/
