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


//http://elixir.free-electrons.com/linux/v4.6/source/include/linux/efi.h#L117
//http://elixir.free-electrons.com/linux/v4.6/source/arch/x86/platform/efi/efi.c

#define EFI_PAGE_SHIFT		12
#define EFI_PAGE_SIZE		(1UL << EFI_PAGE_SHIFT)

typedef struct {
	uint32_t type;
	uint32_t pad1;
	uint32_t phys_addr_a;
	uint32_t phys_addr_b;
	uint32_t virt_addr_a;
	uint32_t virt_addr_b;
	uint32_t num_pages_a;
	uint32_t num_pages_b;
	uint32_t attribute_a;
	uint32_t attribute_b;
	uint64_t pad2;
} efi_memory_desc_t;


struct efi_table_header{
	uint32_t signature_low;
	uint32_t signature_high;
	uint32_t revision;
	uint32_t size;
	uint32_t crc32;
	uint32_t reserved;
};

struct efi_system_table{
	struct efi_table_header eth;
	uint8_t* firmware_vendor;
	uint32_t firmware_revision;
	uintptr_t console_stdin_handle;
	uintptr_t con_in;
	uintptr_t console_stdout_handle;
	uintptr_t con_out;
	uintptr_t console_stderr_handle;
	uintptr_t con_err;
	uintptr_t runtime_services;
	uintptr_t boot_services;
	uint64_t num_of_entries;
	uintptr_t efi_config_table;
};

void efi_parse_est32(uintptr_t est_ptr);