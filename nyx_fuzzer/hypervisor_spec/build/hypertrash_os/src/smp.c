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

#include "smp.h"
#include "apic.h"
#include "tty.h"
#include "mmio.h"
#include "libk.h"
#include "mem.h"

#define STR_PREFIX	" [SMP]  "

uint32_t stack_offset = 0x1; 

volatile uintptr_t ap_stack;

static volatile uint32_t ap_ready = 0;
static uint32_t aps = 0;


void smp_increase_aps(void){
	aps++;
}

void smp_boot_ack(void){
	ap_ready++;
}

void smp_init(void){
	extern void __ap_boot();
	memcpy((void*)AP_INIT_ADDRESS, __ap_boot, 0x1000);

	uint32_t base = apic_get_base();
	printf(STR_PREFIX" Starting %d APs...\n", aps);

	uint32_t volatile tmp = 0;
	uint32_t volatile fetch = 0;

	tmp = ap_ready;
	for(uint32_t i = 1; i < aps; i++){
		ap_stack = mem_alloc_ap_stack();
		fetch = ap_ready;

		mmio_write32(base, LAPIC_ICRHI, i << ICR_DESTINATION_SHIFT);
		mmio_write32(base, LAPIC_ICRLO, ICR_ASSERT | ICR_INIT);

		microdelay(1000000);
		microdelay(1000000);
		mmio_write32(base, LAPIC_ICRHI, i << ICR_DESTINATION_SHIFT);
		mmio_write32(base, LAPIC_ICRLO, ICR_ASSERT | ICR_STARTUP | (AP_INIT_ADDRESS>>12));

		microdelay(1000000);
		mmio_write32(base, LAPIC_ICRHI, i << ICR_DESTINATION_SHIFT);
		mmio_write32(base, LAPIC_ICRLO, ICR_ASSERT | ICR_STARTUP | (AP_INIT_ADDRESS>>12));

		microdelay(1000000);

		if ((mmio_read32(base, LAPIC_ICRLO) & ICR_SEND_PENDING) != 0) {
			printf(STR_PREFIX" AP-WAKENING FAIL!\n");
		}

		while(1){
			fetch = ap_ready;
			if(fetch != tmp){
				tmp = fetch;
				break;
			}
		}
		printf(STR_PREFIX" Processor %d ready...\n", ap_ready);
	}
	printf(STR_PREFIX" Processor 0 ready...\n");
}
