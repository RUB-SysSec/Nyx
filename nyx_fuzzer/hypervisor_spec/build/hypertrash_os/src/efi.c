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

#include "efi.h"
#include "tty.h"

#define STR_PREFIX	" [EFI]  "

static void efi_puts(uintptr_t con_out, uintptr_t str, uint32_t len){
	uint32_t (*efi_printf)(uintptr_t, uintptr_t, uint32_t) __attribute__((cdecl)) = (uint32_t (*)(uintptr_t, uintptr_t, uint32_t))(*((uintptr_t*)(con_out+4)));
	efi_printf(con_out, str, len);
}

void efi_parse_est32(uintptr_t est_ptr){
	struct efi_system_table* est = (struct efi_system_table*)est_ptr;

	printf (STR_PREFIX" eth.signature: %c%c%c%c%c%c%c%c\n", est->eth.signature_low, est->eth.signature_low>>8,
															est->eth.signature_low>>16, est->eth.signature_low>>24,
															est->eth.signature_high, est->eth.signature_high>>8,
															est->eth.signature_high>>16, est->eth.signature_high>>24);

	printf (STR_PREFIX" eth.revisio: %x\n", est->eth.revision);
	printf (STR_PREFIX" eth.size: %x\n", est->eth.size);
	printf (STR_PREFIX" eth.crc32: %x\n", est->eth.crc32);

	printf (STR_PREFIX" firmware_vendor: ");
	for(uint8_t i = 0; est->firmware_vendor[i] != 0x00; i +=2){
		printf("%c", est->firmware_vendor[i]);
	}
	printf("\n");

	printf (STR_PREFIX" firmware_revision: %x\n", est->firmware_revision);
	printf (STR_PREFIX" console_stdin_handle: %x\n", est->console_stdin_handle);
	printf (STR_PREFIX" con_in: %x\n", est->con_in);
	printf (STR_PREFIX" console_stdout_handle: %x\n", est->console_stdout_handle);
	printf (STR_PREFIX" con_out: %x\n", est->con_out);
	printf (STR_PREFIX" console_stderr_handle: %x\n", est->console_stderr_handle);
	printf (STR_PREFIX" con_err: %x\n", est->con_err);
	printf (STR_PREFIX" runtime_services: %x\n", est->runtime_services);
	printf (STR_PREFIX" boot_services: %x\n", est->boot_services);
	printf (STR_PREFIX" num_of_entries: %x\n", est->num_of_entries);
	printf (STR_PREFIX" efi_config_table: %x\n", est->efi_config_table);

	if(est->con_out){
		/* This is necessary to boot a kernel in EFI mode using VMware Fusion.
		 * Don't execute ExitBootServices()! Instead, set our own IDT, GDT 
		 * and treat boot service memory as usable memory. Don't forget to 
		 * disable interrupts first. 
		 */
		printf (STR_PREFIX" bootservices still available!\n");
		efi_puts(est->con_out, (uintptr_t) u"\r\nHypertrash: Hello World from UEFI Land :-]\r\n", 46);

		printf (STR_PREFIX" disabling loader interrupts!\n");
		asm volatile("cli\n");


		printf (STR_PREFIX" flush cr0, cr3, cr4!\n");
		asm volatile (
			"movl %%cr0, %%eax\n"
			"movl $0x1,  %%eax\n"
			"movl %%eax, %%cr0\n"
			"movl $0x0,  %%eax\n"
			"movl %%eax, %%cr3\n"
			"movl $0x0,  %%eax\n"
			"movl %%eax, %%cr4\n"
			::: "%eax");
	}
}