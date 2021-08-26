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

#define RSD_PTR_STR	0x2052545020445352ULL
#define FACP_STR 0x50434146UL
#define SSDT_STR 0x54445353UL
#define APIC_STR 0x43495041UL
#define HPET_STR 0x54455048UL
#define MCFG_STR 0x4746434dUL
#define DSDT_STR 0x54445344UL

struct rsdt_desc {
	uint8_t sig[4];
	uint32_t len;
	uint8_t rev;
	uint8_t chksum;
	uint8_t oem[6];
	uint8_t oem_table[8];
	uint32_t oem_rev;
	uint32_t creator_id;
	uint32_t creator_rev;
} __attribute__ ((packed));

struct mcfg_data_desc {
	uint32_t bar_ecam_lo;
	uint32_t bar_ecam_hi;
	uint16_t pci_seg_group;
	uint8_t pci_start;
	uint8_t pci_end;
	uint32_t reserved;
} __attribute__ ((packed));

struct mcfg_desc {
	struct rsdt_desc acpi;
	uint64_t reserved;
} __attribute__ ((packed));

/* lapic */
struct apic_0_desc {
	uint8_t type;
	uint8_t len;
	uint8_t processor_id;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__ ((packed));

/* ioapic */
struct apic_1_desc {
	uint8_t type;
	uint8_t len;
	uint8_t io_apic_id;
	uint8_t reserved;
	uint32_t io_apic_address;
	uint32_t global_sys_intr_base;
} __attribute__ ((packed));

/* Interrupt Source Override */
struct apic_2_desc {
	uint8_t type;
	uint8_t len;
	uint8_t bus;
	uint8_t source;
	uint32_t global_sys_intr;
	uint16_t flags;
} __attribute__ ((packed));

/* Local APIC NMI */
struct apic_4_desc {
	uint8_t type;
	uint8_t len;
	uint8_t acpi_processor_id;
	uint16_t flags;
	uint8_t lapic_lint;
} __attribute__ ((packed));


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


struct facp_desc{
   uint8_t 	sig[4];
   uint32_t len;
   uint8_t 	ignore_1[40 - 8];
   uint32_t *dsdt_ptr;
   uint8_t 	ignore_2[48 - 44];
   uint32_t *SMI_CMD;
   uint8_t 	ACPI_ENABLE;
   uint8_t 	ACPI_DISABLE;
   uint8_t 	ignore_3[64 - 54];
   uint32_t *PM1a_CNT_BLK;
   uint32_t *PM1b_CNT_BLK;
   uint8_t 	ignore_4[89 - 72];
   uint8_t 	PM1_CNT_LEN;
};

struct apic_desc {
	struct rsdt_desc acpi;
	uint32_t lapic_address;
	uint32_t flags;
	/* APIC structure data */
	/* PCAT_COMPAT uint8_t bit1 */
} __attribute__ ((packed));


struct rsdp_desc {
	uint8_t sig[8];
	uint8_t chksum;
	uint8_t oem[6];
	uint8_t rev;
	uint32_t rsdt_ptr;
} __attribute__ ((packed));

struct rsdp_desc_20 {
	struct rsdp_desc firstPart;
	uint32_t len;
	uint64_t xsdt_ptr;
	uint8_t ext_chksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

void apci_set_rsdp_addr(uintptr_t address);
void acpi_init(void);
