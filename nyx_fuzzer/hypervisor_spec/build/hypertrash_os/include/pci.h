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

#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include "fuzz.h"

#define PCI_CONF_VENDOR_ID_16       0x00
#define PCI_CONF_DEVICE_ID_16       0x02
#define PCI_CONF_COMMAND_16         0x04
#define PCI_CONF_STATUS_16          0x06

#define PCI_CONF_REVISION_ID_8      0x08
#define PCI_CONF_PINTERF_8          0x09
#define PCI_CONF_SUBCLASS_8         0x0a
#define PCI_CONF_CLASS_8            0x0b


#define PCI_CONF_CACHE_LINE_SIZE_8  0x0c
#define PCI_CONF_LATENCY_TIMER_8    0x0d
#define PCI_CONF_HEADER_TYPE_8      0x0e
#define PCI_CONF_BIST_8             0x0f

#define PCI_CONF_BAR_0_32			0x10
#define PCI_CONF_BAR_1_32			0x14
#define PCI_CONF_BAR_2_32			0x18
#define PCI_CONF_BAR_3_32			0x1c
#define PCI_CONF_BAR_4_32			0x20
#define PCI_CONF_BAR_5_32			0x24
#define PCI_CONF_EXP_ROM_32			0x30

#define PCI_CONF_CPTR_8				0x34

#define PCI_CONF_INT_LINE_8  		0x3c
#define PCI_CONF_INT_PIN_8  		0x3d
#define PCI_CONF_MIN_GRANT_8  		0x3e
#define PCI_CONF_MAX_LAT_8  		0x3f

#define PCI_IO_CONFIG_ADDRESS		0xcf8
#define PCI_IO_CONFIG_DATA			0xcfc

#define PCI_NO_DEVICE				0xffff
#define PCI_MAX_BUS					0xff
#define PCI_MAX_SLOT				0x20
#define PCI_MAX_BAR					0x6

struct msi_cap {
	uint8_t cap_id;
	uint8_t next_cap;
	uint16_t message_control;
};

struct msi_cap32 {
	uint8_t cap_id;
	uint8_t next_cap;
	uint16_t message_control;
	uint32_t message_address;
	uint16_t message_data;
};

struct msi_cap64 {
	uint8_t cap_id;
	uint8_t next_cap;
	uint16_t message_control;
	uint32_t message_address;
	uint32_t message_address_upper;
	uint16_t message_data;
};

struct msi_cap32_vec {
	uint8_t cap_id;
	uint8_t next_cap;
	uint16_t message_control;
	uint32_t message_address;
	uint16_t message_data;
	uint16_t reserved;
	uint32_t mask_bits;
	uint32_t pending_bits;
};

struct msi_cap64_vec {
	uint8_t cap_id;
	uint8_t next_cap;
	uint16_t message_control;
	uint32_t message_address;
	uint32_t message_address_upper;
	uint16_t message_data;
	uint16_t reserved;
	uint32_t mask_bits;
	uint32_t pending_bits;
};


struct pci_express_cap {
	uint8_t pcie_cap_id;
	uint8_t next_cap;
	uint16_t pcie_cap_register;
	uint32_t device_cap;
	uint16_t device_control;
	uint16_t device_status; 
};

typedef struct {
	uint32_t address;
	uint32_t size;
	uint8_t type; 
	uint8_t int_pin;
	uint8_t int_line;
} pci_bar_t;

typedef struct {
	uint8_t bus;
	uint8_t slot;
	uint8_t func;
	uint16_t vendor;
	uint16_t product;
	uint8_t class;
	uint8_t subclass;
	uint8_t interface;
	uint32_t num_bars;
	pci_bar_t** bars;

	uint32_t configuration_space_addr;
	bool ecam;
} pci_device_t;

typedef struct {
	uint32_t num_devices;
	pci_device_t* devices[256];
} pci_state_t;
	

pci_state_t* pci_state_new(void);

void set_pci_ecam_address(uintptr_t address);
void pci_state_enum(pci_state_t* self);
void pci_register_areas(pci_state_t* self, fuzzer_state_t* fuzzer);
#endif