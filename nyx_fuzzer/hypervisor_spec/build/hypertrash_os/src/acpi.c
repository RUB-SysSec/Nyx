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
#include "acpi.h"
#include "tty.h"
#include "pci.h"
#include "cpuid.h"
#include "smp.h"
#include "apic.h"
#include "fuzz.h"

uintptr_t acpi_ptr = (uintptr_t)NULL;

#define STR_PREFIX	" [ACPI] "

#define ACPI_APIC_TYPE_L_APIC		0x0
#define ACPI_APIC_TYPE_IO_APIC		0x1
#define ACPI_APIC_TYPE_INT_SO		0x2
#define ACPI_APIC_TYPE_NMI_S		0x3
#define ACPI_APIC_TYPE_LAPIC_NMI	0x4
#define ACPI_APIC_TYPE_LAPIC_O		0x5
#define ACPI_APIC_TYPE_IO_SAPIC		0x6
#define ACPI_APIC_TYPE_L_SAPIC		0x7
#define ACPI_APIC_TYPE_P_INTR_S		0x8
#define ACPI_APIC_TYPE_PL_2xAPIC	0x9
#define ACPI_APIC_TYPE_L_2xAPIC_NMI	0xa

static void acpi_apic_type_0_parse(uintptr_t apic_ptr_offset){
	struct apic_0_desc* data = (struct apic_0_desc*)apic_ptr_offset;
	printf(STR_PREFIX"               processor_id %1x\n\r", data->processor_id);
	printf(STR_PREFIX"               apic_id %1x\n\r", data->apic_id);
	printf(STR_PREFIX"               enabled %1x\n\r", !!(data->flags));
	smp_increase_aps();
}

static void acpi_apic_type_1_parse(uintptr_t apic_ptr_offset){
	struct apic_1_desc* data = (struct apic_1_desc*)apic_ptr_offset;
	printf(STR_PREFIX"               io_apic_id %1x\n\r", data->io_apic_id);
	printf(STR_PREFIX"               io_apic_address %4x\n\r", data->io_apic_address);
	printf(STR_PREFIX"               global_sys_intr_base %4x\n\r", data->global_sys_intr_base);
	apic_set_ioapic_base(data->io_apic_address);
}

static void acpi_apic_type_2_parse(uintptr_t apic_ptr_offset){
	struct apic_2_desc* data = (struct apic_2_desc*)apic_ptr_offset;
	printf(STR_PREFIX"               bus %1x\n\r", data->bus);
	printf(STR_PREFIX"               source %1x\n\r", data->source);
	printf(STR_PREFIX"               global_sys_intr %4x\n\r", data->global_sys_intr);
	printf(STR_PREFIX"               flags %2x\n\r", data->flags);
	apic_ioapic_irq_remap(data->source, data->global_sys_intr);
}

static void acpi_apic_type_4_parse(uintptr_t apic_ptr_offset){
	struct apic_4_desc* data = (struct apic_4_desc*)apic_ptr_offset;
	printf(STR_PREFIX"               acpi_processor_id %1x\n\r", data->acpi_processor_id);
	printf(STR_PREFIX"               flags %2x\n\r", data->flags);
	printf(STR_PREFIX"               lapic_lint %1x\n\r", data->lapic_lint);
}

static void acpi_apic_parse(uintptr_t apic_ptr){
	uint8_t type, len;
	uint16_t checksum_value = 0;
	struct apic_desc* data = (struct apic_desc*)apic_ptr;

	for(uint32_t i = 0; i < data->acpi.len; i++){
		checksum_value += ((uint8_t*)apic_ptr)[i];
	}

	if (!(checksum_value & 0xff)){

		printf(STR_PREFIX"         APIC lapic_address: %x\n\r", data->lapic_address);
		printf(STR_PREFIX"         APIC flags: %x\n\r", data->flags);

		printf(STR_PREFIX"         APIC len: %d\n\r", data->acpi.len);
		printf(STR_PREFIX"         APIC oem: ");
		for(uint8_t i = 0; i < 6; i++){
			printf("%c", data->acpi.oem[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"         APIC oem_table: ");
		for(uint8_t i = 0; i < 8; i++){
			printf("%c", data->acpi.oem_table[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"         APIC oem_rev: %x\n\r", data->acpi.oem_rev);
		printf(STR_PREFIX"         APIC creator_id: %x\n\r", data->acpi.creator_id);
		printf(STR_PREFIX"         APIC creator_rev: %x\n\r", data->acpi.creator_rev);

		for(uint16_t i = 44; i < data->acpi.len; i += *((uint8_t*)(apic_ptr+i+1))){
			type = *((uint8_t*)(apic_ptr+i));
			len = *((uint8_t*)(apic_ptr+(i+1)));
			printf(STR_PREFIX"            type: %d len: %d\n\r", type, len);
			switch(type){
				case ACPI_APIC_TYPE_L_APIC: 		acpi_apic_type_0_parse(apic_ptr+i); break;
				case ACPI_APIC_TYPE_IO_APIC: 		acpi_apic_type_1_parse(apic_ptr+i); break;
				case ACPI_APIC_TYPE_INT_SO: 		acpi_apic_type_2_parse(apic_ptr+i); break;
				case ACPI_APIC_TYPE_NMI_S:			break;
				case ACPI_APIC_TYPE_LAPIC_NMI:		acpi_apic_type_4_parse(apic_ptr+i); break;
				case ACPI_APIC_TYPE_LAPIC_O:		break;
				case ACPI_APIC_TYPE_IO_SAPIC:		break;
				case ACPI_APIC_TYPE_L_SAPIC:		break;
				case ACPI_APIC_TYPE_P_INTR_S:		break;
				case ACPI_APIC_TYPE_PL_2xAPIC:		break;
				case ACPI_APIC_TYPE_L_2xAPIC_NMI:	break;
			}
		}
	}
}

static void acpi_facp_parse(uintptr_t facp_ptr){
	struct facp_desc* data = (struct facp_desc*)facp_ptr;
	printf(STR_PREFIX"         FACP len:		  %x\n\r", data->len);
	printf(STR_PREFIX"         FACP dsdt_ptr:	  %x\n\r", data->dsdt_ptr);

	for(uintptr_t i = (uintptr_t)data->dsdt_ptr; i < (uintptr_t)data->dsdt_ptr+0x1000; i++){
		if (*((uint32_t*)i) == 0x5f35535f ){
			printf(STR_PREFIX"         Found \"_S5_\" at %x!\n\r", i-(uintptr_t)(data->dsdt_ptr));

			break;
		}
	}

	printf(STR_PREFIX"         FACP SMI_CMD:	  %x\n\r", data->SMI_CMD);
	printf(STR_PREFIX"         FACP ACPI_ENABLE:  %x\n\r", data->ACPI_ENABLE);
	printf(STR_PREFIX"         FACP ACPI_DISABLE: %x\n\r", data->ACPI_DISABLE);

	printf(STR_PREFIX"         FACP PM1a_CNT_BLK: %x\n\r", data->PM1a_CNT_BLK);
	printf(STR_PREFIX"         FACP PM1b_CNT_BLK: %x\n\r", data->PM1b_CNT_BLK);
	printf(STR_PREFIX"         FACP PM1_CNT_LEN:  %x\n\r", data->PM1_CNT_LEN);
}

static void acpi_mcfg_parse(uintptr_t mcfg_ptr){
	uint16_t checksum_value = 0;
	uint16_t entries = 0;
	struct mcfg_desc* data = (struct mcfg_desc*)mcfg_ptr;

	for(uint8_t i = 0; i < data->acpi.len; i++){
		checksum_value += ((uint8_t*)mcfg_ptr)[i];
	}
	if (!(checksum_value & 0xff)){
		entries = (data->acpi.len-sizeof(struct mcfg_desc)) / 16;
		printf(STR_PREFIX"         MCFG len: %d [Entries: %d]\n\r", data->acpi.len, entries);
		printf(STR_PREFIX"         MCFG oem: ");
		for(uint8_t i = 0; i < 6; i++){
			printf("%c", data->acpi.oem[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"         MCFG oem_table: ");
		for(uint8_t i = 0; i < 8; i++){
			printf("%c", data->acpi.oem_table[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"         MCFG oem_rev: %x\n\r", data->acpi.oem_rev);
		printf(STR_PREFIX"         MCFG creator_id: %x\n\r", data->acpi.creator_id);
		printf(STR_PREFIX"         MCFG creator_rev: %x\n\r", data->acpi.creator_rev);

		for(uint16_t i = 0; i < entries; i++){
			struct mcfg_data_desc* mcfg_data = (struct mcfg_data_desc*)(mcfg_ptr + (sizeof(struct mcfg_desc)) + (16*i));
			set_pci_ecam_address(mcfg_data->bar_ecam_lo); /* 32bit hack */
			printf(STR_PREFIX"            MCFG%d bar: 0x%4x%4x\n\r", i, mcfg_data->bar_ecam_hi, mcfg_data->bar_ecam_lo);
			printf(STR_PREFIX"            MCFG%d pci_seg_group: 0x%2x\n\r", i, mcfg_data->pci_seg_group);
			printf(STR_PREFIX"            MCFG%d start: 0x%1x\n\r", i, mcfg_data->pci_start);
			printf(STR_PREFIX"            MCFG%d end:   0x%1x\n\r", i, mcfg_data->pci_end);
		}
	}
}

static void acpi_misc_parse(uintptr_t misc_ptr){
	uint8_t* data = (uint8_t*)misc_ptr;
	for(uint8_t i = 0; i < 4; i++){
			printf("%c", data[i]);
	}
	printf("\n\r")

	switch(*((uint32_t*)data)){
		case APIC_STR:	acpi_apic_parse(misc_ptr); break;			/* Multiple APIC Description Table 			*/
		/* BOOT */																						/* http://www.feishare.com/attachments/081_SBF21.pdf */
		/* DBGP */
		/* DSDT */
		/* ECDT */
		/* ETDT */
		case FACP_STR:	acpi_facp_parse(misc_ptr); break;			/* Fixed ACPI Description Table 			*/
		/* FACS */
		case HPET_STR:	break;																/* High Precision Event Time 				*/
		case MCFG_STR:	acpi_mcfg_parse(misc_ptr); break;			/* MCFG / MMIO Configuration (PCIe ECAM)	*/
		/* OEMx */
		/* PSDT */
		/* RSDT */
		/* SBST */
		/* SLIT */
		/* SPCR */
		/* SRAT */
		case SSDT_STR:	break;																/* Secondary System Description Table 		*/
		/* SPMI */
		/* XSDT */
	}
}

/* Root System Description Table */
static void acpi_rsdt_parse(uintptr_t rsdt_ptr){
	uint16_t checksum_value = 0;
	struct rsdt_desc* data = (struct rsdt_desc*)rsdt_ptr;

	for(uint8_t i = 0; i < data->len; i++){
		checksum_value += ((uint8_t*)rsdt_ptr)[i];
	}
	if (!(checksum_value & 0xff)){
		printf(STR_PREFIX"      RSDT len: %d [Entries: %d]\n\r", data->len, (data->len-sizeof(struct rsdt_desc)) / 4);
		printf(STR_PREFIX"      RSDT oem: ");
		for(uint8_t i = 0; i < 6; i++){
			printf("%c", data->oem[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"      RSDT oem_table: ");
		for(uint8_t i = 0; i < 8; i++){
			printf("%c", data->oem_table[i]);
		}
		printf("\n\r");
		printf(STR_PREFIX"      RSDT oem_rev: %x\n\r", data->oem_rev);
		printf(STR_PREFIX"      RSDT creator_id: %x\n\r", data->creator_id);
		printf(STR_PREFIX"      RSDT creator_rev: %x\n\r", data->creator_rev);

		for(uint32_t i = 0; i < (data->len-sizeof(struct rsdt_desc)) / 4; i++){
			printf(STR_PREFIX"      -> 0x%x ", ((uint32_t*)(rsdt_ptr + sizeof(struct rsdt_desc)))[i]);
			acpi_misc_parse(((uint32_t*)(rsdt_ptr + sizeof(struct rsdt_desc)))[i]);
		}
	}
}


static void acpi_rsdp_parse(uintptr_t rsd_ptr){
	uint16_t checksum_value = 0;
	struct rsdp_desc* data = (struct rsdp_desc*)rsd_ptr;
	for(uint8_t i = 0; i < sizeof(struct rsdp_desc); i++){
		checksum_value += ((uint8_t*)rsd_ptr)[i];
	}
	if (!(checksum_value & 0xff)){
		printf(STR_PREFIX"   RSDP oem: ");
		for(uint8_t i = 0; i < 6; i++){
			printf("%c", data->oem[i]);
		}
		printf("\n\r");
		switch(data->rev){
			case 0:		printf(STR_PREFIX"   RSDP rev: ACPI 1.0\n\r"); break;
			case 2:		printf(STR_PREFIX"   RSDP rev: ACPI 2.0\n\r"); break;
			default:	printf(STR_PREFIX"   RSDP rev: %2x\n\r", data->rev); break;
		}
		printf(STR_PREFIX"   RSDP ptr: %x\n\r", data->rsdt_ptr);
		acpi_rsdt_parse((uintptr_t) data->rsdt_ptr);
	}
}

static bool acpi_init_bios(void){
	for(uint32_t i = 0x000A0000; i < 0x000FFFFF; i+=8){
		if(RSD_PTR_STR == *((uint64_t*)i)){ /* "RSD PTR " */
			printf(STR_PREFIX" RSDP: 0x%x\n\r", i);
			apci_set_rsdp_addr(i);
			acpi_rsdp_parse((uintptr_t) i);
			return true;
		}
	}
	return false;
}

static bool acpi_init_uefi(void){
	/* todo */
	return false;
}

void apci_set_rsdp_addr(uintptr_t address){
	acpi_ptr = address;
}


void acpi_init(void){
	if(acpi_ptr){
		printf(STR_PREFIX" using multiboot2 rsdp ptr...\n\r");
		acpi_rsdp_parse(acpi_ptr);
		return;
	}

	if(acpi_init_bios()){
		return;
	}
	if(acpi_init_uefi()){
		return;
	}

	printf(STR_PREFIX" not supported...\n\r");
}

void acpi_register_areas(fuzzer_state_t* fuzzer){
	if(acpi_ptr){
 	   register_area(fuzzer, acpi_ptr, 0x1000, 1, "ACPI TABLE");
 	}
}