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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define UNUSED(x) (void)(x)

#define SYSCALL_VECTOR 0x7F

struct regs {
	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss;
};

typedef struct regs regs_t;

typedef void (*irq_handler_t) (struct regs *);

#define PAUSE   { asm volatile ("hlt"); }
#define STOP while (1) { PAUSE; }

/* GDT */
extern void gdt_install(void);
extern void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);

/* IDT */
extern void idt_install(void);
extern void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags);

/* ISRS */
extern void isrs_install(void);
extern void isrs_install_handler(size_t isrs, irq_handler_t);
extern void isrs_uninstall_handler(size_t isrs);

extern void* kernel_end;