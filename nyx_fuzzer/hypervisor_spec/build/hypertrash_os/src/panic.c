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

#include "kafl_user.h"
#include "panic.h"
#include "tty.h"
#include "io.h"
#include "config.h"

#define FAULT_DIV_BY_ZERO	0
#define FAULT_DEBUG			1
#define FAULT_NMI			2

static char* panic_reason[] = {
	"Divide-by-zero Error",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved",
	"x87 Floating-Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Reserved",
	"Security Exception",
	"Reserved",
	"Triple Fault",
	"FPU Error Interrupt"
};

#define INT_PAGE_FAULT 0x0E

void panic_isr(struct regs* r) {
	panic_handler(r);
}

#define RDMSR_INS 0x320f
#define WRMSR_INS 0x300f


void reboot2(void) {
	asm volatile ("cli");
    uint8_t good = 0;
    while (good & 0x02){
        good = inb(0x64);
    }
    outb(0xFE, 0x64);
}

#define RECOVER() return;

void* ret_addr = 0;
void set_ret(void* addr){
	ret_addr = addr;
}

void panic_handler(struct regs* r) {
	uint32_t cr0, cr2, cr3, cr4;

#ifndef PAYLOAD
	kAFL_hypercall(HYPERCALL_KAFL_NESTED_EARLY_RELEASE, 0);
	kAFL_hypercall(HYPERCALL_KAFL_NESTED_RELEASE, 0);
#endif

	printf("\n\rKernel Panic :-(\n\r\n\r");
	printf("RETURN %x -> %x\n", r->eip, ret_addr);
	printf("\n\rKernel Panic :-(\n\r\n\r");
	printf("CPU crashed with int %d and error code %d.\n\r", r->int_no, r->err_code);
	printf("Reason: %s ", panic_reason[r->int_no]);
	if(r->int_no == INT_PAGE_FAULT){
		asm volatile("mov %%cr2, %0" : "=r"(cr2));
		printf("(Faulting Address 0x%x)", cr2);
	}
	printf("\n\rFaulting EIP 0x%x\n\r\n\r", r->eip);

	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	asm volatile("mov %%cr2, %0" : "=r"(cr2));
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	printf("Registers:\n\r");
	printf("---------------------------------------------------------------\n\r");
	printf("GS:  0x%x FS:  0x%x ES:  0x%x DS:  0x%x\n\r", r->gs, r->es, r->es, r->ds);
	printf("EDI: 0x%x ESI: 0x%x EBP: 0x%x ESP: 0x%x\n\r",  r->edi, r->esi, r->ebp, r->esp);
	printf("EBX: 0x%x EDX: 0x%x ECX: 0x%x EAX: 0x%x\n\r", r->ebx, r->edx, r->ecx, r-> eax);
	printf("CS:  0x%x SS:  0x%x EFLAGS:              0x%x\n\r", r->cs, r->ss, r->eflags);

	printf("CR0: 0x%x CR2: 0x%x CR3: 0x%x CR4: 0x%x\n\r\n\r", cr0, cr2, cr3, cr4);
	r->eip = (uintptr_t)ret_addr;
	r->eflags &= 0xFFFFFEFF;

#ifndef PAYLOAD
	kAFL_hypercall(HYPERCALL_KAFL_NESTED_EARLY_RELEASE, 0);
#endif
    
	tty_initialize(CRASH_COLOR);
	printf("\n\rKernel Panic :-(\n\r\n\r");
	printf("CPU crashed with int %d and error code %d.\n\r", r->int_no, r->err_code);
	printf("Reason: %s ", panic_reason[r->int_no]);
	if(r->int_no == INT_PAGE_FAULT){
		asm volatile("mov %%cr2, %0" : "=r"(cr2));
		printf("(Faulting Address 0x%x)", cr2);
	}
	printf("\n\rFaulting EIP 0x%x\n\r\n\r", r->eip);

	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	asm volatile("mov %%cr2, %0" : "=r"(cr2));
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	printf("Registers:\n\r");
	printf("---------------------------------------------------------------\n\r");
	printf("GS:  0x%x FS:  0x%x ES:  0x%x DS:  0x%x\n\r", r->gs, r->es, r->es, r->ds);
	printf("EDI: 0x%x ESI: 0x%x EBP: 0x%x ESP: 0x%x\n\r",  r->edi, r->esi, r->ebp, r->esp);
	printf("EBX: 0x%x EDX: 0x%x ECX: 0x%x EAX: 0x%x\n\r", r->ebx, r->edx, r->ecx, r-> eax);
	printf("CS:  0x%x SS:  0x%x EFLAGS:              0x%x\n\r", r->cs, r->ss, r->eflags);

	printf("CR0: 0x%x CR2: 0x%x CR3: 0x%x CR4: 0x%x\n\r\n\r", cr0, cr2, cr3, cr4);

	printf("Halting CPU...\n\r");

	/* halt CPU */
	STOP;
	 kAFL_hypercall(HYPERCALL_KAFL_NESTED_RELEASE, 0);
}
