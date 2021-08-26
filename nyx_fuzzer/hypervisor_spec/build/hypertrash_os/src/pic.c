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

#include <stdint.h>
#include "pic.h"
#include "io.h"
#include "tty.h"

#define STR_PREFIX	" [PIC]  "

static bool enabled = false;

static uint16_t pic_get_irq_reg(int ocw3){
    outb(PIC1_CMD, ocw3);
    outb(PIC2_CMD, ocw3);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

static uint16_t pic_get_irr(void){
    return pic_get_irq_reg(PIC_READ_IRR);
}

static inline void pic_irq_master_eoi(void){
	outb(PIC_EOI, PIC1_CMD);
}

static inline void pic_irq_slave_eoi(void){
	outb(PIC_EOI, PIC2_CMD);
}

void pic_irq_ack(uint8_t irq){
	pic_irq_master_eoi();
	if (irq >= 8) {
		pic_irq_slave_eoi();
	}
}

bool pci_interrupt_pending(uint8_t irq){
	return !!(pic_get_irr() & (1<<irq));
}

bool pic_spurious_irq(uint8_t irq){
    if(!enabled){
        return false;
    }

	if (irq == 15 || irq == 7){
		if(!(pic_get_irr() & (1<<irq))){
			printf(STR_PREFIX" spurious IRQ %d\n", irq);
			if((irq) == 15){
				pic_irq_master_eoi();
			}
		return true;
		}
	}
	return false;
}

void pic_eoi(uint8_t irq){
    if(enabled){
        pic_irq_ack(irq);
    }
}

void pic_disable(void){
    asm volatile("cli\n");
    enabled = false;
    outb(PIC_DISABLE, PIC1_DATA);
    outb(PIC_DISABLE, PIC2_DATA);
}

void pic_init(void){
    enabled = true;

    outb(ICW1_INIT | ICW1_ICW4, PIC1_CMD);
    outb(ICW1_INIT | ICW1_ICW4, PIC2_CMD);

    outb(IRQ_BASE, PIC1_DATA);
    outb(IRQ_BASE + 8, PIC2_DATA);

    outb(4, PIC1_DATA);
    outb(2, PIC2_DATA);

    outb(ICW4_8086, PIC1_DATA);
    outb(ICW4_8086, PIC2_DATA);

    outb(0x00, PIC1_DATA);
    outb(0x00, PIC2_DATA);
    printf(STR_PREFIX" intel 8259 reconfigured!\n");

    printf(STR_PREFIX" interrupts enabled!\n");
    asm volatile("sti\n");
}