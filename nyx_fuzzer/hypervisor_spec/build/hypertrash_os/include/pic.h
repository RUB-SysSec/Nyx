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
#include <stdbool.h>

#define IRQ_BASE                        0x20

#define PIC1_CMD                        0x0020
#define PIC1_DATA                       0x0021
#define PIC2_CMD                        0x00a0
#define PIC2_DATA                       0x00a1

#define ICW1_ICW4                       0x01  
#define ICW1_SINGLE                     0x02
#define ICW1_ADI                        0x04 
#define ICW1_LTIM                       0x08 
#define ICW1_INIT                       0x10

#define ICW4_8086                       0x01
#define ICW4_AUTO                       0x02
#define ICW4_BUF_SLAVE                  0x04
#define ICW4_BUF_MASTER                 0x0C
#define ICW4_SFNM                       0x10

#define PIC_EOI							0x20

#define PIC_READ_IRR               		0x0a 
#define PIC_READ_ISR               		0x0b

#define PIC_DISABLE 					0xff

void pic_eoi(uint8_t irq);
bool pci_interrupt_pending(uint8_t irq);
void pic_irq_ack(uint8_t irq);
bool pic_spurious_irq(uint8_t irq);
void pic_disable(void);
void pic_init(void);