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

 /* credits: wiki.osdev.org, syssec.rub.de (angry_os) */

#include "system.h"
#include "mem.h"
#include "libk.h"
#include "panic.h"
#include "tty.h"
#include "mboot.h"

#define STR_PREFIX	" [MEM]  "

//#define MEM_DEBUG 
//define MEM_SANITIZER

uintptr_t low_mem = (uintptr_t)0x0;
uintptr_t high_mem = (uintptr_t)0x1FFC00;

uintptr_t e820_mmap = (uintptr_t)NULL;
uint32_t e820_entries = 0x0;

uintptr_t efi_mmap = (uintptr_t)NULL;


uintptr_t alloc_address = (uintptr_t)NULL;

page_directory_t* kernel_directory;

static void set_alloc_start_address(uintptr_t start) {
	printf("%s: %x\n\r", __func__, start);
	alloc_address = (uintptr_t)start;
}

uint32_t *frames;
uint32_t nframes;

#define INDEX_FROM_BIT(b) (b / 0x20)
#define OFFSET_FROM_BIT(b) (b % 0x20)

void set_frame(uintptr_t frame_addr) {
	if (frame_addr < nframes * 4 * 0x400) {
		uint32_t frame  = frame_addr / 0x1000;
		uint32_t index  = INDEX_FROM_BIT(frame);
		uint32_t offset = OFFSET_FROM_BIT(frame);
		frames[index] |= (0x1 << offset);
	}
}

void clear_frame(uintptr_t frame_addr) {
	uint32_t frame  = frame_addr / 0x1000;
	uint32_t index  = INDEX_FROM_BIT(frame);
	uint32_t offset = OFFSET_FROM_BIT(frame);
	frames[index] &= ~(0x1 << offset);
}

uint32_t test_frame(uintptr_t frame_addr) {
	uint32_t frame  = frame_addr / 0x1000;
	uint32_t index  = INDEX_FROM_BIT(frame);
	uint32_t offset = OFFSET_FROM_BIT(frame);
	return (frames[index] & (0x1 << offset));
}

uint32_t first_n_frames(int n) {
	for (uint32_t i = 0; i < nframes * 0x1000; i += 0x1000) {
		int bad = 0;
		for (int j = 0; j < n; ++j) {
			if (test_frame(i + 0x1000 * j)) {
				bad = j+1;
			}
		}
		if (!bad) {
			return i / 0x1000;
		}
	}
	return 0xFFFFFFFF;
}

uint32_t first_frame(void) {
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(nframes); ++i) {
		if (frames[i] != 0xFFFFFFFF) {
			for (j = 0; j < 32; ++j) {
				uint32_t testFrame = 0x1 << j;
				if (!(frames[i] & testFrame)) {
					return i * 0x20 + j;
				}
			}
		}
	}
	return 0xFFFFFFFF;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
	if (page->frame != 0) {
		page->present = 1;
		page->rw      = (is_writeable == 1) ? 1 : 0;
		page->user    = (is_kernel == 1)    ? 0 : 1;
		return;
	} else {
		//spin_lock(frame_alloc_lock);
		uint32_t index = first_frame();
		//assert(index != (uint32_t)-1 && "Out of frames.");
		set_frame(index * 0x1000);
		page->frame   = index;
		//spin_unlock(frame_alloc_lock);
		page->present = 1;
		page->rw      = (is_writeable == 1) ? 1 : 0;
		page->user    = (is_kernel == 1)    ? 0 : 1;
	}
}
uint8_t paging_enabled = 0;


void dma_frame(page_t *page, int is_kernel, int is_writeable, uintptr_t address) {
	if(paging_enabled){
		 asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :
        :"%eax");
	}
	/* Page this address directly */
	page->present = (is_writeable) ? 1 : 0;
	page->rw      = (is_writeable) ? 1 : 0;
	page->user    = (is_kernel)    ? 0 : 1;
	page->frame   = address / 0x1000;

	if(paging_enabled){
		 asm volatile(
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :
        :"%eax");
	}
	set_frame(address);
}

void free_frame(page_t *page) {
	uint32_t frame;
	if (!(frame = page->frame)) {
		//assert(0);
		return;
	} else {
		clear_frame(frame * 0x1000);
		page->frame = 0x0;
	}
}

uintptr_t memory_use(void ) {
	uintptr_t ret = 0;
	uint32_t i, j;
	for (i = 0; i < INDEX_FROM_BIT(nframes); ++i) {
		for (j = 0; j < 32; ++j) {
			uint32_t testFrame = 0x1 << j;
			if (frames[i] & testFrame) {
				ret++;
			}
		}
	}
	return ret * 4;
}

uintptr_t memory_total(){
	return nframes * 4;
}

static inline void switch_page_directory(page_directory_t* dir) {
	asm volatile (
			"mov %0, %%cr3\n\r"
			"mov %%cr0, %%eax\n\r"
			"orl $0x80000000, %%eax\n\r"
			"mov %%eax, %%cr0\n\r"
			:: "r"(dir->physical_address)
			: "%eax");
}

static inline void enable_write_protection(void){
	asm volatile (
      "movl %%cr0, %%eax\n\r"
      "orl $0x10000, %%eax\n\r"
			"mov %%eax, %%cr0\n\r"
			:::);
}

static inline void write_physmem_8(uintptr_t address, uint8_t value){
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %0, %%eax\n\r;"
        "movb %1, (%%eax)\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :"r"(address), "r"(value)
        :"%eax", "%edi", "%esi", "%ebp");
}

static void* memset_physmem(void* dst, int c, size_t len) {
	if(paging_enabled){
		for(size_t i = 0; i < len; i++) {
			write_physmem_8((uintptr_t)(dst + i), c);
		}
		return dst;
	}
	else{
		memset(dst, c, len);
	}
	return dst;
}

page_t* get_page(uintptr_t address, int make, page_directory_t* dir, bool writeable) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, address);
#endif
	address /= 0x1000;
	uint32_t table_index = address / 1024;
	if(dir->tables[table_index]) {
		return &dir->tables[table_index]->pages[address % 1024];
	}
	else if(make) {
		//uint32_t temp = 0;
		//dir->tables[table_index] = (page_table_t *)pvmalloc_p(sizeof(page_table_t), (uintptr_t *)(&temp));

		uintptr_t temp = (uintptr_t)pvmalloc(sizeof(page_table_t));
		dir->tables[table_index] = (page_table_t *)temp;


		//memset(dir->tables[table_index], 0, sizeof(page_table_t));
		memset_physmem(dir->tables[table_index], 0, sizeof(page_table_t));
		if(writeable){
			dir->physical_tables[table_index] = (uintptr_t)(temp | 0x7); /* Present, R/w, User */ 
		}
		else{
			dir->physical_tables[table_index] = (uintptr_t)(temp | 0x4); /* Present, R, User */ 
		}
		return &dir->tables[table_index]->pages[address % 1024];
	} else {
		return 0;
	}
}

static void paging_prepare(uint32_t memsize) {
	nframes = memsize  / 0x1000;
#ifdef MEM_DEBUG
	printf(STR_PREFIX" allocating ~%d KB (nframes)\n\r", (INDEX_FROM_BIT(nframes * 8) >> 10));
#endif
	frames  = (uint32_t *)pmalloc(INDEX_FROM_BIT(nframes * 8));

	memset(frames, 0, INDEX_FROM_BIT(nframes * 8));

	kernel_directory = (page_directory_t *)pvmalloc(sizeof(page_directory_t));
#ifdef MEM_DEBUG
	printf("%s 0x%x-0x%x\n\r", __func__, kernel_directory, ((uintptr_t)kernel_directory+sizeof(page_directory_t)));
#endif
	memset(kernel_directory, 0, sizeof(page_directory_t));

}


static void paging_install() {
	get_page(0, 1, kernel_directory, true)->present = 0;
	set_frame(0);

	/* Direct Mapping of Kernel text / stack section + Lowmem (VGA, BIOS, ...) */
	for (uintptr_t i = 0; i < (uintptr_t)&kernel_end; i += 0x1000) {
		dma_frame(get_page(i & 0xFFFFF000, 1, kernel_directory, true), 1, 1, i & 0xFFFFF000);
	}

	/* Direct Mapping of APIC and IOAPIC memory regions */
	for (uintptr_t i = 0xfec00000; i < 0xfef00000; i += 0x1000) {
		dma_frame(get_page(i & 0xFFFFF000, 1, kernel_directory, true), 1, 1, i & 0xFFFFF000);
	}

	/* Direct Mapping kernel PD */
	for (uintptr_t i = 0; i < 0x10000; i += 0x1000) {
		dma_frame(get_page(((uintptr_t)kernel_directory)+i, 1, kernel_directory, true), 1, 1, ((uintptr_t)kernel_directory)+i);	
	}	

	kernel_directory->physical_address = (uintptr_t)kernel_directory->physical_tables;

	switch_page_directory(kernel_directory);
#ifdef MEM_DEBUG
	printf(STR_PREFIX " new cr3 value is %x\n\r", kernel_directory);
#endif
	paging_enabled = 1;
}

uintptr_t mem_alloc_ap_stack(void){
	uintptr_t ptr;
#ifdef MEM_DEBUG
	printf("Try to alloc\n\r");
#endif
	ptr = (uintptr_t) kmalloc(0x1000);
#ifdef MEM_DEBUG
	printf(STR_PREFIX "Done: %x", ptr);
#endif
	dma_frame(get_page(ptr & 0xFFFFF000, 1, kernel_directory, true), 1, 1, ptr & 0xFFFFF000);

	return ptr;
}

void kfree(void* page){
	UNUSED(page);
	// ...
}

uintptr_t pmalloc_real(size_t size, int align, uintptr_t* phys) {

	/* fix alignment if required */
	if(align && alloc_address & 0xfffff000) {
		alloc_address &= 0xfffff000;
		alloc_address += 0x1000;
	}

	if(phys) {
		*phys = alloc_address;
	}

	uintptr_t ret = alloc_address;

	alloc_address += size;
	return ret;
}

uintptr_t remap_area = 0x820000;

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE-1))

#define PAGE_ALIGN(addr)        (((addr)+PAGE_SIZE-1)&PAGE_MASK)

void* virtual_map(uintptr_t page, size_t size){

	uintptr_t return_value = remap_area;

	if(!size){
		return 0;
	}

	size = PAGE_ALIGN(size);

	for(uint32_t i = 0; i < size / 0x1000; i++){
		uintptr_t tpage = (uintptr_t)get_page(remap_area & 0xFFFFF000, 1, kernel_directory, true);
		dma_frame((void*)tpage, 1, 1, page & 0xFFFFF000);
		remap_area += 0x1000;
		page += 0x1000;
	}

	return (void*)return_value;

}

void* kmalloc_real(size_t size, int align, uintptr_t* phys) {
#ifdef MEM_SANITIZER
	UNUSED(align);
	void* return_value = (void*)pmalloc_real(size+0x1000, 1, phys);
#else
	void* return_value = (void*)pmalloc_real(size, align, phys);
#endif

#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, return_value);
#endif
	//while(1){};
	for(uint32_t i = 0; i <= (size/4096); i++){
#ifdef MEM_DEBUG
		printf("DMA MAPPING 0x%x\n\r", return_value+(i*4096));
#endif
		dma_frame(get_page((uintptr_t)(return_value+(i*4096)), 1, kernel_directory, true), 1, 1, (uintptr_t)(return_value+(i*4096)));
	}

#ifdef MEM_SANITIZER
	/* guard page */
	dma_frame(get_page((uintptr_t)(return_value+(((size/4096)+1)*4096)), 1, kernel_directory, false), 1, 0, (uintptr_t)(return_value+(((size/4096)+1)*4096)));
	return return_value + (0x1000-(size%0x1000));
#else
	return return_value;
#endif
}


uint32_t counter_pmalloc = 0;
uint32_t counter_pvmalloc = 0;

uint32_t counter_kvmalloc_p = 0;
uint32_t counter_kmalloc_p = 0;
uint32_t counter_kvmalloc = 0;
uint32_t counter_kmalloc = 0;

/* allocate physical page */
void* pmalloc(size_t size) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_pmalloc++;
	return (void*)pmalloc_real(size, 0, NULL);
}

/* allocate physical page aligned */
void* pvmalloc(size_t size) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_pvmalloc++;
	return (void*)pmalloc_real(size, 1, NULL);
}


void* kmalloc(size_t size) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_kmalloc++;
	return kmalloc_real(size, 0, NULL);	/* don't ask :| */
}

void* kvmalloc(size_t size) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_kvmalloc++;
	return kmalloc_real(size, 1, NULL);
}

void* kmalloc_p(size_t size, uintptr_t* phys) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_kmalloc_p++;
	return kmalloc_real(size, 0, phys);
}

void* kvmalloc_p(size_t size, uintptr_t* phys) {
#ifdef MEM_DEBUG
	printf("%s %x\n\r", __func__, size);
#endif
	counter_kvmalloc_p++;
	return kmalloc_real(size, 1, phys);
}

void print_mem_stats(void){
	printf("\n\r-------------------------\n\r");
	printf(STR_PREFIX " MEMSTATS:\n\r");
	printf(STR_PREFIX " kmalloc: %d\n\r", counter_kmalloc);
	printf(STR_PREFIX " kvmalloc: %d\n\r", counter_kvmalloc);
	printf(STR_PREFIX " kmalloc_p: %d\n\r", counter_kmalloc_p);
	printf(STR_PREFIX " kvmalloc_p: %d\n\r", counter_kvmalloc_p);
	printf("\n\r");
	printf(STR_PREFIX " pmalloc: %d\n\r", counter_pmalloc);
	printf(STR_PREFIX " pvmalloc: %d\n\r", counter_pvmalloc);



	printf(STR_PREFIX " kmalloc: %d\n\r", counter_kmalloc+counter_kvmalloc+counter_kmalloc_p+counter_kvmalloc_p);
	printf(STR_PREFIX " pmalloc: %d\n\r", counter_pmalloc+counter_pvmalloc);

}

void mem_set_low_high(uintptr_t mboot_low_mem, uintptr_t mboot_high_mem){
	low_mem = mboot_low_mem;
	high_mem = mboot_high_mem;
}

void mem_set_e820_mmap(uintptr_t e820_mmap_ptr, uint32_t entries){
	e820_mmap = e820_mmap_ptr;
	e820_entries = entries;
}

void mem_set_efi_mmap(uintptr_t efi_mmap_ptr){
	efi_mmap = efi_mmap_ptr;
}

static void mem_e820_parse(void){
	struct multiboot_mmap_entry* tmp = (struct multiboot_mmap_entry*)e820_mmap;
	for(uint32_t i = 0; i < e820_entries; i++){
		if (tmp[i].type != 1) {
			for (uint64_t i = 0; i < tmp[i].len_a; i += 0x1000) {
				if (tmp[i].addr_a + i > 0xFFFFFFFF) break;
				set_frame((tmp[i].addr_a + i) & 0xFFFFF000);
			}
		}
	}
}

void mem_set_ap_cr3(void){
	switch_page_directory(kernel_directory);
}

bool mem_init(void){
	bool mmap_configured = false;

	printf (STR_PREFIX" setup memory management...\n\r");
	printf(STR_PREFIX " Low:  %x\n\r", low_mem * 0x400);
	printf(STR_PREFIX " High: %x\n\r", high_mem * 0x400);
	printf(STR_PREFIX " End:  %x\n\r", &kernel_end);
	printf(STR_PREFIX " Total:  %d MB\n\r", (low_mem+high_mem)/0x400);

	if((uintptr_t)&kernel_end > (low_mem * 0x400)){
		set_alloc_start_address((uintptr_t)&kernel_end);
	}else{
		set_alloc_start_address((uintptr_t)(low_mem * 0x400));
	}

	paging_prepare((low_mem + high_mem) * 0x400);
	printf (STR_PREFIX" enabling paging...\n\r");
	if(efi_mmap){
		//mmap_configured = true;
	}
	if(e820_mmap && !mmap_configured){
		mem_e820_parse();
		mmap_configured = true;
	}
	else{
		for (uint64_t i = 0x00100000; i < 0x1f3defff; i += 0x1000) {
			set_frame(i);
		}
	}
	if(!mmap_configured){
		printf (STR_PREFIX" mmap not found...\n\r");
		//return false;
	}

	paging_install();
	enable_write_protection();

	return true;
}

uintptr_t slab_start = 0;
uintptr_t slab_pos = 0;
uintptr_t slab_end = 0;

int init_slab_allocator(uint32_t size){

	size &= 0xFFFFF000;
	if(!size){
		/* fail */
		return 0;
	}

	slab_start = (uintptr_t)kvmalloc(size);
	slab_end = slab_start + size;
	slab_pos = slab_start;
	return 1;
}

uintptr_t slab_alloc(size_t size){
	if(slab_pos + size < slab_end){
			uintptr_t ret_value = slab_pos;
			slab_pos += size;
			return ret_value;
	}
	return 0; /* error handling ? */
}

uintptr_t slab_alloc_page_aligend(size_t size){

	if((slab_pos&0xFFF) && ((slab_pos&0xFFFFF000) + 0x1000 + size) < slab_end){
		uintptr_t ret_value = (slab_pos&0xFFFFF000) + 0x1000;
		slab_pos = (slab_pos&0xFFFFF000) + 0x1000 + size;
		return ret_value;
	}

	if((slab_pos&0xFFF) == 0 && slab_pos + size < slab_end){
			uintptr_t ret_value = slab_pos;
			slab_pos += size;
			return ret_value;
	}

	return 0;
}
