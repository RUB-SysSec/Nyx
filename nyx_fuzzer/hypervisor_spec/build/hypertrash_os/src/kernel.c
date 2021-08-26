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

#include "system.h"
#include "libk.h"
#include "mboot.h"
#include "serial.h"
#include "mem.h"
#include "vga.h"
#include "tty.h"
#include "pci.h"
#include "panic.h"
#include "io.h"
#include "msr.h"
#include "test.h"
#include "acpi.h"
#include "cpuid.h"
#include "apic.h"
#include "pic.h"
#include "smp.h"
#include "isa.h"
#include "mmio.h"
#include "kafl_user.h"
#include "fuzz.h"

#include "config.h"

extern uintptr_t pci_ecam_ptr;

#define STR_PREFIX	" [KERN] "

extern uint8_t _binary_misc_logo_bmp_start;

fuzzer_state_t* fuzzer;

static inline void kernel_init_console(void){
#ifndef PAYLOAD
	/* in case of triple fault reboots */
	kAFL_hypercall(HYPERCALL_KAFL_NESTED_EARLY_RELEASE, 0);
#endif
	tty_initialize(DEFAULT_COLOR);
	serial_enable(SERIAL_PORT_A);
	tty_setcolor(SUCCESS_COLOR);
	printf(STR_PREFIX" Booting HyperCube OS Kernel...\n\r");
	serial_printf(SERIAL_PORT_A, " Booting HyperCube OS Kernel...\n\r");
	return;

	serial_printf(SERIAL_PORT_A,"-----------------------------------------\n\r");
	serial_printf(SERIAL_PORT_A,"                 #######           \n\r");
	serial_printf(SERIAL_PORT_A,"             ####       ####       \n\r");
	serial_printf(SERIAL_PORT_A,"           ##               ##     \n\r");
	serial_printf(SERIAL_PORT_A,"         ##                   ##   \n\r");
	serial_printf(SERIAL_PORT_A,"        #                       #  \n\r");
	serial_printf(SERIAL_PORT_A,"       #  ####        ######     # \n\r");
	serial_printf(SERIAL_PORT_A,"      #  #   ##      #    ###     #\n\r");
	serial_printf(SERIAL_PORT_A,"      # #      #     #       #    #\n\r");
	serial_printf(SERIAL_PORT_A,"      # #########    #########    #\n\r");
	serial_printf(SERIAL_PORT_A,"      #                           #\n\r");
	serial_printf(SERIAL_PORT_A,"      #                           #\n\r");
	serial_printf(SERIAL_PORT_A,"      #   ####################    #\n\r");
	serial_printf(SERIAL_PORT_A,"       #   #        #####   #    # \n\r");
	serial_printf(SERIAL_PORT_A,"       #    #     ##    ## ##    # \n\r");
	serial_printf(SERIAL_PORT_A,"        #    ##  #        ##    #  \n\r");
	serial_printf(SERIAL_PORT_A,"         ##    ##       ##    ##   \n\r");
	serial_printf(SERIAL_PORT_A,"           ##    #######    ##     \n\r");
	serial_printf(SERIAL_PORT_A,"             ###         ###       \n\r");
	serial_printf(SERIAL_PORT_A,"                #########          \n\r");
	serial_printf(SERIAL_PORT_A,"-----------------------------------------\n\r\n\r");

	tty_setcolor(DEFAULT_COLOR);
}

static inline void kernel_init_interrupts(void){
	/* setup ISRs */
	printf (STR_PREFIX" installing GDT\n\r");
	gdt_install();
	printf (STR_PREFIX" installing IDT\n\r");
	idt_install();
	printf (STR_PREFIX" installing ISRs\n\r");
	isrs_install();

	printf (STR_PREFIX" installing default ISR handlers\n\r");
	/* install default handler for ISRs */
	for(size_t i = 0; i < 32; i++) {
		isrs_install_handler(i, panic_handler);
	}
	return;
	isrs_install_handler(SYSCALL_VECTOR, panic_handler);

}

static void kernel_ready(void){
	tty_setcolor(DEFAULT_COLOR);

	tty_drawbmp(&_binary_misc_logo_bmp_start);
	tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BROWN));
	tty_setpos(54, 24);
	tty_writestring(" HyperCube OS ");

	tty_setpos(0, 24);
	return;
	while(1){
		asm("hlt\n\r");
	}
}

void ap_main(void){
	mem_set_ap_cr3();

	asm volatile(
        "movl $0x5000, %esp\n\r"
    );

	gdt_install();
	smp_boot_ack();
	while(1){
		asm volatile("hlt\n\r");
	}
}


void kernel_main(struct multiboot_tag* mbi, uint32_t mboot_magic, uint32_t foo) {
	kernel_init_console();

	printf(STR_PREFIX" mbi: %x...\n\r", mbi);
	printf(STR_PREFIX" mboot_magic: %x...\n\r", mboot_magic);
	printf(STR_PREFIX" foo: %x...\n\r", foo);


	disable_printf();

	if(!mboot_init(mbi, mboot_magic)){
		goto fail;
	}
	kernel_init_interrupts();

	print_cpuid();

	acpi_init();
	pic_init();
	apic_init();
	
  if(!mem_init()){
    goto fail;
  }

	init_hprintf();


  smp_init();

  pci_state_t* pci_state = pci_state_new();
  pci_state_enum(pci_state);

	print_mem_stats();

	fuzzer = new_fuzzer();

	if(fuzzer){
		
#ifdef IO_FUZZING
			isa_state_enum(fuzzer);		
#endif

#ifdef PCI_FUZZING	
			pci_register_areas(pci_state, fuzzer);
#endif

#ifdef APIC_FUZZING
			apic_register_areas(fuzzer);
#endif

#ifdef HPET_FUZZING
			register_area(fuzzer, 0xFed00000, 0x1000, 1, "HPET");
#endif
		
		print_mem_stats();

		uint8_t* payload_buffer = prepare_fuzzing(fuzzer);
		kernel_ready();

		start_fuzzing(payload_buffer, (64 * 1024));
	}
	printf("Shutdown...\n");
	
	outw(0x2000, 0x604);
	while(1){
		asm("hlt\n\r");
	}

	fail:
	tty_setcolor(FAIL_COLOR);
}
