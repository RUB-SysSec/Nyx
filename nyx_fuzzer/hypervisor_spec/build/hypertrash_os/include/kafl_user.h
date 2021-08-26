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

#ifndef KAFL_USER_H
#define KAFL_USER_H

#ifndef HYPERTRASH
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#else
#include "libk.h"
#include "tty.h"
#endif

#include <stdarg.h>
#ifndef __MINGW64__
#include <sys/mman.h>
#endif

#ifdef __MINGW64__
#ifndef uint64_t
#define uint64_t UINT64
#endif
#ifndef int32_t
#define int32_t INT32
#endif
#ifndef uint8_t
#define uint8_t UINT8
#endif
#else 
#include <stdint.h>
#endif

//#define HYPERCALL_KAFL_RAX_ID				0xBADC0FFE	/* new kAFL hypercall ID! */


#define HYPERCALL_KAFL_RAX_ID                           0x1f      /* new kAFL hypercall ID! */

#define HYPERCALL_KAFL_ACQUIRE				0
#define HYPERCALL_KAFL_GET_PAYLOAD			1
#define HYPERCALL_KAFL_GET_PROGRAM			2
#define HYPERCALL_KAFL_GET_ARGV				3
#define HYPERCALL_KAFL_RELEASE				4
#define HYPERCALL_KAFL_SUBMIT_CR3			5
#define HYPERCALL_KAFL_SUBMIT_PANIC			6
#define HYPERCALL_KAFL_SUBMIT_KASAN			7
#define HYPERCALL_KAFL_PANIC				8
#define HYPERCALL_KAFL_KASAN				9
#define HYPERCALL_KAFL_LOCK					10
#define HYPERCALL_KAFL_INFO					11
#define HYPERCALL_KAFL_NEXT_PAYLOAD			12
#define HYPERCALL_KAFL_PRINTF				13
#define HYPERCALL_KAFL_PRINTK_ADDR			14
#define HYPERCALL_KAFL_PRINTK				15

/* user space only hypercalls */
#define HYPERCALL_KAFL_USER_RANGE_ADVISE	16
#define HYPERCALL_KAFL_USER_SUBMIT_MODE		17
#define HYPERCALL_KAFL_USER_FAST_ACQUIRE	18
/* 19 is already used for exit reason KVM_EXIT_KAFL_TOPA_MAIN_FULL */
#define HYPERCALL_KAFL_USER_ABORT			20

/* hypertrash only hypercalls */
#define HYPERTRASH_HYPERCALL_MASK			0xAA000000

#define HYPERCALL_KAFL_NESTED_PREPARE		(0 | HYPERTRASH_HYPERCALL_MASK)
#define HYPERCALL_KAFL_NESTED_CONFIG		(1 | HYPERTRASH_HYPERCALL_MASK)
#define HYPERCALL_KAFL_NESTED_ACQUIRE		(2 | HYPERTRASH_HYPERCALL_MASK)
#define HYPERCALL_KAFL_NESTED_RELEASE		(3 | HYPERTRASH_HYPERCALL_MASK)
#define HYPERCALL_KAFL_NESTED_HPRINTF		(4 | HYPERTRASH_HYPERCALL_MASK)
#define HYPERCALL_KAFL_NESTED_EARLY_RELEASE		(5 | HYPERTRASH_HYPERCALL_MASK)


#define PAYLOAD_SIZE						(128 << 10)				/* up to 128KB payloads */
#define PROGRAM_SIZE						(128 << 20)				/* kAFL supports 128MB programm data */
#define INFO_SIZE        					(128 << 10)				/* 128KB info string */
#define TARGET_FILE							"/tmp/fuzzing_engine"	/* default target for the userspace component */
#define TARGET_FILE_WIN						"fuzzing_engine.exe"	

#define HPRINTF_MAX_SIZE					0x1000					/* up to 4KB hprintf strings */


typedef struct{
	int32_t size;
	uint8_t data[PAYLOAD_SIZE-sizeof(int32_t)];
} kAFL_payload;

typedef struct{
	uint64_t ip[4];
	uint64_t size[4];
	uint8_t enabled[4];
} kAFL_ranges; 

#define KAFL_MODE_64	0
#define KAFL_MODE_32	1
#define KAFL_MODE_16	2

#if defined(__i386__)
static inline void kAFL_hypercall(uint32_t ebx, uint32_t ecx){
# ifndef __NOKAFL
	uint32_t eax = HYPERCALL_KAFL_RAX_ID;
	/* fix me ! */
	/*
	asm("pushl %%ecx;" : : );
	asm("pushl %%ebx;" : : );
    asm("pushl %%eax;" : : );
	asm("movl %0, %%ecx;" : : "r"(rcx));
	asm("movl %0, %%ebx;" : : "r"(rbx));
    asm("movl %0, %%eax;" : : "r"(rax));
    asm("vmcall");
    asm("popl %%eax;" : : );
	asm("popl %%ebx;" : : );
    asm("popl %%ecx;" : : );
    */
    asm volatile("movl %0, %%ecx;"
				 "movl %1, %%ebx;"  
				 "movl %2, %%eax;"
				 "vmcall" 
				: 
				: "r" (ecx), "r" (ebx), "r" (eax) 
				: "eax", "ecx", "ebx"
				);

# endif
} 
#elif defined(__x86_64__)

static void kAFL_hypercall(uint64_t rbx, uint64_t rcx){
# ifndef __NOKAFL
	uint64_t rax = HYPERCALL_KAFL_RAX_ID;
    asm volatile("movq %0, %%rcx;"
				 "movq %1, %%rbx;"  
				 "movq %2, %%rax;"
				 "vmcall" 
				: 
				: "r" (rcx), "r" (rbx), "r" (rax) 
				: "rax", "rcx", "rbx"
				);
# endif
}
#endif

#ifndef HYPERTRASH
static uint8_t* hprintf_buffer = NULL; 

static inline uint8_t alloc_hprintf_buffer(void){
	if(!hprintf_buffer){
#ifdef __MINGW64__
		hprintf_buffer = (uint8_t*)VirtualAlloc(0, HPRINTF_MAX_SIZE, MEM_COMMIT, PAGE_READWRITE);
#else 
		hprintf_buffer = mmap((void*)NULL, HPRINTF_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
		if(!hprintf_buffer){
			return 0;
		}
	}
	return 1; 
}

#ifdef __NOKAFL
int (*hprintf)(const char * format, ...) = printf;
#else
static void hprintf(const char * format, ...)  __attribute__ ((unused));

static void hprintf(const char * format, ...){
	va_list args;
	va_start(args, format);
	if(alloc_hprintf_buffer()){
		vsnprintf((char*)hprintf_buffer, HPRINTF_MAX_SIZE, format, args);
# if defined(__i386__)
		printf("%s", hprintf_buffer);
		kAFL_hypercall(HYPERCALL_KAFL_PRINTF, (uint32_t)hprintf_buffer);
# elif defined(__x86_64__)
		printf("%s", hprintf_buffer);
		kAFL_hypercall(HYPERCALL_KAFL_PRINTF, (uint64_t)hprintf_buffer);
# endif
	}
	//vprintf(format, args);
	va_end(args);
}
#endif
#endif
#endif

#ifdef HYPERTRASH
static uint8_t* nested_hprintf_buffer = (uint8_t*)0xFFFFFFFF; 

static inline void alloc_nested_hprintf_buffer(void* page){

	nested_hprintf_buffer = (uint8_t*) ((uintptr_t)page &0xFFFFF000);
	//printf("%s: %x\n", __func__, nested_hprintf_buffer);
}

static __attribute__((unused)) void nested_hprintf(const char* format, ...){
	va_list args;
	va_start(args, format);
	if((uintptr_t)nested_hprintf_buffer != 0xFFFFFFFF){
		vsnprintf((char*)nested_hprintf_buffer, 0x1000, format, args);
		//printf("---> %s\n", nested_hprintf_buffer);
		kAFL_hypercall(HYPERCALL_KAFL_NESTED_HPRINTF, (uint32_t)nested_hprintf_buffer);
	}
}

static __attribute__((unused)) void nested_hprintf_str(const char* buffer){
	if((uintptr_t)nested_hprintf_buffer != 0xFFFFFFFF){
		strncpy((char*)nested_hprintf_buffer, (const char*)buffer, (size_t)0x1000);
		kAFL_hypercall(HYPERCALL_KAFL_NESTED_HPRINTF, (uint32_t)nested_hprintf_buffer);
	}
}
#endif