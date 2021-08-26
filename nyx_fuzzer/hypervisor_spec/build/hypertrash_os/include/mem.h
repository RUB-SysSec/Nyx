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

typedef struct page {
	unsigned int present:1;
	unsigned int rw:1;
	unsigned int user:1;
	unsigned int writethrough:1;
	unsigned int cachedisable:1;
	unsigned int unused:7;
	unsigned int frame:20;
} __attribute__((packed)) page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory {
	uintptr_t physical_tables[1024];	/* Physical addresses of the tables */
	page_table_t *tables[1024];			/* 1024 pointers to page tables... */
	uintptr_t physical_address;			/* The physical address of physical_tables */
	int32_t ref_count;
} page_directory_t;

void kfree(void* page);

void* malloc(size_t size);

void* kmalloc(size_t size);
void* kvmalloc(size_t size);
void* kmalloc_p(size_t size, uintptr_t* phys);
void* kvmalloc_p(size_t size, uintptr_t* phys);

void* pmalloc(size_t size);
void* pvmalloc(size_t size);
void* pmalloc_p(size_t size, uintptr_t* phys);
void* pvmalloc_p(size_t size, uintptr_t* phys);


void mem_set_low_high(uintptr_t mboot_low_mem, uintptr_t mboot_high_mem);
void mem_set_e820_mmap(uintptr_t e820_mmap_ptr, uint32_t entries);
void mem_set_efi_mmap(uintptr_t efi_mmap_ptr);

uintptr_t mem_alloc_ap_stack(void);
void mem_set_ap_cr3(void);
bool mem_init(void);
void* virtual_map(uintptr_t page, size_t size);

void print_mem_stats(void);