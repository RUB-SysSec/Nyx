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
#include "tty.h"
#include "io.h"
#include "pic.h"
#include "mmio.h"
#include "apic.h"
#include "isr.h"

static uint8_t msi_slot = 0; 

static irq_handler_t isr_routines[256] = { 0 };

static volatile uint8_t is_fuzzer_ready = 0;

uint8_t fuzzer_ready(void){
	return is_fuzzer_ready;
}

void flush_fuzzer_ready(void){
	is_fuzzer_ready = 0;
}

void irq_handler(struct regs *r) {
	uint8_t irq = r->int_no-32;
	uint8_t key = 0;


	if(!pic_spurious_irq(irq)){
		/* Time Interrupt */
		if (irq != 0){
			//printf("IRQ %d\n", (r->int_no-32));
		}

		/* Keyboard Interrupt */
		if(irq == 1){
			key = inb(0x60);
			if (key >= 0x1 && key < 0x5){
				printf("is_fuzzer_ready: %lx\n", key-0x1);
				is_fuzzer_ready = key-0x1;
			}
		}
		
		if(irq == 11){
			//test = (uint32_t*)(0x810000 + (rand()%0x1000));
			//test = (uint32_t*)*test;
		}
	
		apic_eoi(irq);
		pic_eoi(irq);
	}
}

uint8_t alloc_msi_vectors(uint8_t vnum){
	uint8_t tmp;
	if(!vnum){
		return 0xff;
	}
	if (vnum+msi_slot < 200){
		tmp = msi_slot;
		msi_slot += vnum;
		return tmp;
	}
	else{
		return 0xff;
	}
}

void msi_irq_handler(struct regs *r) {
	uint8_t irq = r->int_no-32;
	//printf(" [IRQ]   MSI 0x%1x\n", (r->int_no-32-24));
	apic_eoi(irq);
}

void isrs_install_handler(size_t isrs, irq_handler_t handler) {
	isr_routines[isrs] = handler;
}

void isrs_uninstall_handler(size_t isrs) {
	isr_routines[isrs] = 0;
}

void isrs_install(void) {
	/* exception handlers 0-31*/
	for(uint8_t i = 0; i < 32; i++){
		idt_set_gate(i, (void*)isrs[i], 0x08, 0x8E);
	}

	/* legacy irqs 32-56 */
	for(uint8_t i = 0; i < 16; i++){
		idt_set_gate(i+32, (void*)irqs[i], 0x08, 0x8E);
	}

	/* MSI interrupts 57 - 254 */
	for(uint8_t i = 0; i < 199; i++){
		idt_set_gate(i+32+24, (void*)msi_irqs[i], 0x08, 0x8E);
	}

	idt_set_gate(0xff, (void*)_irq_spurious, 0x08, 0x8E);
}

void fault_handler(struct regs * r) {
	irq_handler_t handler = isr_routines[r->int_no];
	if (handler) {
		handler(r);
	} else {
		STOP;
	}
}
