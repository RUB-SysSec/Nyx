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

#include <stdbool.h>
#include <stdint.h>
#include "apic.h"
#include "cpuid.h"
#include "tty.h"
#include "msr.h"
#include "mmio.h"
#include "pic.h"
#include "fuzz.h"

#define STR_PREFIX	" [APIC] "

uint8_t irq_remapping[] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};

static uint32_t apic_base = 0;
static uint32_t ioapic_base = 0;
static bool enabled = false;

static void apic_set_base(uint32_t base){
	apic_base = base;
}

void apic_ioapic_irq_remap(uint8_t src, uint8_t dst){
	irq_remapping[src%0x10] = dst;
	irq_remapping[dst%0x10] = src;
}

uint32_t apic_get_base(void){
	return apic_base;
}

uint32_t apic_get_io_base(void){
	return ioapic_base;
}

void apic_set_ioapic_base(uint32_t base){
	ioapic_base = base;
}

static uintptr_t apic_enable(uintptr_t base){
   uint32_t high = 0;
   uint32_t low = (base & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;
 
   wrmsr32(IA32_APIC_BASE, low, high);
   return base & 0xfffff000;
}

static inline uintptr_t apic_read_msr_base(void){
	return (uintptr_t)rdmsr64(IA32_APIC_BASE);
}

static inline bool apic_check_supported(void){
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);
	return (CHECK(edx, f_edx_bit_APIC));
}

static void IoApicOut(uintptr_t base, uint8_t reg, uint32_t val)
{
    mmio_write32(base, IOREGSEL, reg);
    mmio_write32(base, IOWIN, val);
}

void IoApicSetEntry(uintptr_t io_apic_addr, uint8_t index, uint64_t data)
{
    IoApicOut(io_apic_addr, IOREDTBL + index * 2, (uint32_t)data);
    IoApicOut(io_apic_addr, IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
}

uint32_t apic_get_local_id(void){
	uint32_t base = apic_get_base();
	if(base){
		return mmio_read32(base, LAPIC_ID) >> 24;
	}
	return 0;
}

static void io_apic_init(void){
	if (!ioapic_base){
		printf(STR_PREFIX" IOAPIC base not found ...\n\r");
		return;
	}

	printf(STR_PREFIX" disabling PIC!\n\r");
	pic_disable();
	asm volatile("cli\n\r");

	mmio_write32(ioapic_base, IOREGSEL, IOAPICVER);
	uint32_t x = mmio_read32(ioapic_base, IOWIN);

    uint32_t count = ((x >> 16) & 0xff) + 1;

    printf(STR_PREFIX" I/O APIC pins = %d\n\r", count);

    // Disable all entries
    for (uint32_t i = 0; i < count; ++i)
    {
        IoApicSetEntry(ioapic_base, i, 1 << 16);
    }

    for(uint8_t i = 0; i < 16; i++){
    	IoApicSetEntry(ioapic_base, irq_remapping[i], 32+i);		
	}

	//    IoApicSetEntry(ioapic_base, irq_remapping[2], 32+0);		
	//    IoApicSetEntry(ioapic_base, irq_remapping[1], 32+1);	
	//    IoApicSetEntry(ioapic_base, irq_remapping[0xa], 32+0xa);		

    asm volatile("sti\n\r");
}


void apic_eoi(uint8_t irq){
	(void)irq;
	//hexdump("apic_base: ", apic_get_base() | (0 << 12), 128);

	if(enabled){
		mmio_write32(apic_get_base(), 0xb0, 0);
	}
	//hexdump("apic_base: ", apic_get_base() | (0 << 12), 128);
}

bool apic_init(void){
	uintptr_t base;

	if(apic_check_supported()){
		printf(STR_PREFIX" supported!\n\r");

		base = apic_read_msr_base();
		base = apic_enable(base);
		apic_set_base(base);
		printf(STR_PREFIX" base is 0x%x\n\r", base);
		//mmio_write16(base, 0xF0, (mmio_read16(base, 0xF0) | 0x100));

		mmio_write32(base, LAPIC_TPR, 0);
		mmio_write32(base, LAPIC_DFR, 0xffffffff);
		mmio_write32(base, LAPIC_LDR, 0x01000000);
		mmio_write32(base, LAPIC_SVR, 0x100 | 0xff);

		printf(STR_PREFIX" LAPIC ID: %2x\n\r", apic_get_local_id());

		enabled = true;
		io_apic_init();
		return true;

	}
	printf(STR_PREFIX" not supported...\n\r");
	return false;
}

void apic_register_areas(fuzzer_state_t* fuzzer){
	uint32_t base = apic_get_base();
	if(base){
 	   register_area(fuzzer, base, 0x100000, 1, "LAPIC");
 	}
 	uint32_t io_base = apic_get_io_base();
 	if(io_base){
 	   register_area(fuzzer, io_base, 0x1000, 1, "IOAPIC");
 	}
}