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

#include "tty.h"
#include "serial.h"
#include "io.h"
#include "pci.h"
#include "apic.h"
#include "isr.h"
#include "mem.h"
#include "fuzz.h"
#include <stdbool.h>
#include "config.h"

#define STR_PREFIX  " [PCI]  "

uintptr_t pci_ecam_ptr = (uintptr_t)NULL;

static inline uint8_t read_physmem_8(uintptr_t address){
    uint8_t result = 0;
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %1, %%eax\n\r;"
        "movb (%%eax), %0\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :"=r"(result)
        :"r"(address)
        :"%eax", "%edi", "%esi", "%ebp");

    return result;
}


static inline uint16_t read_physmem_16(uintptr_t address){
    uint16_t result = 0;
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %1, %%eax\n\r;"
        "movw (%%eax), %0\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :"=r"(result)
        :"r"(address)
        :"%eax", "%edi", "%esi", "%ebp");

    return result;
}

static inline uint32_t read_physmem_32(uintptr_t address){
    uint32_t result = 0;
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %1, %%eax\n\r;"
        "movl (%%eax), %0\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :"=r"(result)
        :"r"(address)
        :"%eax", "%edi", "%esi", "%ebp");

    return result;
}

static inline void write_physmem_8(uintptr_t address, uint8_t value){
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %0, %%eax\n\r;"
        "movb %1, (%%eax)\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :"r"(address), "r"(value)
        :"%eax", "%edi", "%esi", "%ebp");
}

static inline void write_physmem_16(uintptr_t address, uint16_t value){
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %0, %%eax\n\r;"
        "movw %1, (%%eax)\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :"r"(address), "r"(value)
        :"%eax", "%edi", "%esi", "%ebp");
}

static inline void write_physmem_32(uintptr_t address, uint32_t value){
    asm volatile(
        "movl %%cr0, %%eax\n\r"
        "andl $0x7FFFFFFF, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        "movl %0, %%eax\n\r;"
        "movl %1, (%%eax)\n\r;"
        "movl %%cr0, %%eax\n\r"
        "orl $0x80000000, %%eax\n\r"
        "movl %%eax, %%cr0\n\r"
        :
        :"r"(address), "r"(value)
        :"%eax", "%edi", "%esi", "%ebp");
}

void* memset_physmem(void* dst, int c, size_t len) {
    for(size_t i = 0; i < len; i++) {
        write_physmem_8((uintptr_t)dst + i, c);
    }
    return dst;
}

void hexdump_physmem(char *desc, uintptr_t addr, uint32_t len) {
    uint32_t i;
    uint8_t buff[17];

    uint8_t tmp = 0;

    if (desc != NULL)
        printf ("%s:\n\r", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n\r");
        return;
    }

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n\r", buff);

            printf ("  %04x ", i);
        }

        
        tmp = read_physmem_8(addr + i);

        printf (" %02x", tmp);

        if ((tmp < 0x20) || (tmp > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = tmp;
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    printf ("  %s\n\r", buff);
}

static void pci_bus_enumeration(pci_state_t* self, uint8_t bus, bool ecam);

static char* get_class_string(uint8_t pci_class){
    switch(pci_class){
        case 0x00: return "Default";
        case 0x01: return "Massstorage";
        case 0x02: return "Network";
        case 0x03: return "Display";
        case 0x04: return "Multimedia";
        case 0x05: return "Memory";
        case 0x06: return "Bridge"; 
        case 0x07: return "Simple communications controllers";
        case 0x08: return "Base system peripherals";
        case 0x09: return "Input devices"; 
        case 0x0a: return "Docking Stations"; 
        case 0x0b: return "Processor"; 
        case 0x0c: return "Serial bus controllers";
        case 0xff: return "Misc"; 
        default: return "Reserved"; 
    }
    return "";
}

#define SUBCLASS(__subclass, __pinterface) ((__subclass << 8) + __pinterface)

static inline char* get_subclass_0x0_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "Non-VGA";
        case SUBCLASS(0x01, 0x00):  return "VGA-Device";
        default:                    return "Unknown"; 
    }
}

static inline char* get_subclass_0x1_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "SCSI";
        case SUBCLASS(0x02, 0x00):  return "Floppy";
        case SUBCLASS(0x03, 0x00):  return "IPI";
        case SUBCLASS(0x04, 0x00):  return "RAID";
        case SUBCLASS(0x05, 0x20):  return "ATA-Controller, Single DMA";
        case SUBCLASS(0x05, 0x30):  return "ATA-Controller, Chained DMA";
        case SUBCLASS(0x06, 0x00):  return "SATA-Controller, Proprietary";
        case SUBCLASS(0x06, 0x01):  return "SATA-Controller, AHCI 1.0";
        case SUBCLASS(0x07, 0x00):  return "SAS-Controller";
        case SUBCLASS(0x08, 0x00):  return "NV-Memory, generic";
        case SUBCLASS(0x08, 0x01):  return "NV-Memory, NVMHCI";
        case SUBCLASS(0x08, 0x02):  return "NV-Memory, Enterprise NVMHCI";
        case SUBCLASS(0x80, 0x00):  return "Other";
    }

    if (pci_subclass == 0x1){
        return "IDE";
    }

    return "Unknown"; 
}

static inline char* get_subclass_0x2_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "Ethernet";
        case SUBCLASS(0x01, 0x00):  return "Token Ring";
        case SUBCLASS(0x02, 0x00):  return "FDDI";
        case SUBCLASS(0x03, 0x00):  return "ATM";
        case SUBCLASS(0x04, 0x00):  return "ISDN";
        case SUBCLASS(0x05, 0x00):  return "World FIP";
        case SUBCLASS(0x80, 0x00):  return "Other";
    }

    if(pci_subclass == 0x06){
        return "PICMG 2.14, Multi-Computing";
    }
    return "Unknown"; 
}

static inline char* get_subclass_0x3_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "VGA";
        case SUBCLASS(0x00, 0x01):  return "8514";
        case SUBCLASS(0x01, 0x00):  return "XGA";
        case SUBCLASS(0x02, 0x00):  return "3D";
        case SUBCLASS(0x80, 0x00):  return "Other";
        default:                    return "Unknown"; 
    }
}

static inline char* get_subclass_0x6_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "Host/PCI-Bridge";
        case SUBCLASS(0x01, 0x00):  return "PCI/ISA-Bridge";
        case SUBCLASS(0x02, 0x00):  return "PCI/EISA-Bridge";
        case SUBCLASS(0x03, 0x00):  return "PCI/Micro Channel-Bridge";
        case SUBCLASS(0x04, 0x00):  return "PCI/PCI-Express Bridge";
        case SUBCLASS(0x04, 0x01):  return "PCI/PCI-Bridge";
        case SUBCLASS(0x05, 0x00):  return "PCI/PCMCIA-Bridge";
        case SUBCLASS(0x06, 0x00):  return "PCI/NuBus-Bridge";
        case SUBCLASS(0x07, 0x00):  return "PCI/CardBus-Bridge";
        case SUBCLASS(0x09, 0x40):  return "ST PCI/PCI-Bridge Primary";
        case SUBCLASS(0x09, 0x80):  return "ST PCI/PCI-Bridge Secondary";
        case SUBCLASS(0x0a, 0x00):  return "InfiniBand/PCI-Brige";
    }

    if(pci_subclass == 0x08){
        return "Raceway Switched Fabric";
    }
    if(pci_subclass == 0x80){
        return "Other";
    }
    return "Unknown"; 
}

static inline char* get_subclass_0x8_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "Generic 8259 PIC";
        case SUBCLASS(0x00, 0x01):  return "ISA PIC";
        case SUBCLASS(0x00, 0x02):  return "EISA PIC";
        case SUBCLASS(0x00, 0x10):  return "I/O APIC Interrupt Controller";
        case SUBCLASS(0x00, 0x20):  return "I/O(x) APIC Interrupt Controller";
        case SUBCLASS(0x01, 0x00):  return "Generic 8237 DMA Controller";
        case SUBCLASS(0x01, 0x01):  return "ISA DMA Controller";
        case SUBCLASS(0x01, 0x02):  return "EISA DMA Controller";
        case SUBCLASS(0x02, 0x00):  return "Generic 8254 System Timer";
        case SUBCLASS(0x02, 0x01):  return "ISA System Timer";
        case SUBCLASS(0x02, 0x02):  return "EISA System Timer";
        case SUBCLASS(0x03, 0x00):  return "Generic RTC Controller";
        case SUBCLASS(0x03, 0x01):  return "ISA RTC Controller";
        case SUBCLASS(0x04, 0x00):  return "Generic PCI Hot-Plug Controller";
        case SUBCLASS(0x80, 0x00):  return "Other System Peripheral";

    }
    return "Unknown"; 
}

static inline char* get_subclass_0xc_string(uint8_t pci_subclass, uint8_t pinterface){
    uint16_t value = (pci_subclass << 8) + pinterface;
    switch(value){
        case SUBCLASS(0x00, 0x00):  return "EEE 1394 Controller FireWire";
        case SUBCLASS(0x00, 0x10):  return "IEEE 1394 Controller OpenHCI";
        case SUBCLASS(0x01, 0x00):  return "ACCESS.bus";
        case SUBCLASS(0x02, 0x00):  return "SSA";
        case SUBCLASS(0x03, 0x00):  return "USB UHCI";
        case SUBCLASS(0x03, 0x10):  return "USB OHCI";
        case SUBCLASS(0x03, 0x20):  return "USB EHCI";
        case SUBCLASS(0x03, 0x30):  return "USB XHCI";
        case SUBCLASS(0x03, 0x80):  return "USB";
        case SUBCLASS(0x04, 0xFE):  return "Fibre Channel";
        case SUBCLASS(0x05, 0x00):  return "SMBus";
        case SUBCLASS(0x06, 0x00):  return "InfiniBand";
        case SUBCLASS(0x07, 0x00):  return "IPMI SMIC";
        case SUBCLASS(0x07, 0x01):  return "IPMI Kybd";
        case SUBCLASS(0x07, 0x02):  return "IPMI Block";
        case SUBCLASS(0x08, 0x00):  return "SERCOS";
        case SUBCLASS(0x09, 0x00):  return "CANbus";
    }
    return "Unknown"; 
}

static char* get_subclass_string(uint8_t pci_class, uint8_t pci_subclass, uint8_t pinterface){
    switch(pci_class){
        case 0x00: return get_subclass_0x0_string(pci_subclass, pinterface);
        case 0x01: return get_subclass_0x1_string(pci_subclass, pinterface);
        case 0x02: return get_subclass_0x2_string(pci_subclass, pinterface);
        case 0x03: return get_subclass_0x3_string(pci_subclass, pinterface);
        case 0x04: return "MEDIA";
        case 0x05: break;
        case 0x06: return get_subclass_0x6_string(pci_subclass, pinterface);
        case 0x07: return "SERIAL";
        case 0x08: return get_subclass_0x8_string(pci_subclass, pinterface);
        case 0x09: return "INPUT";
        case 0x0a: break;
        case 0x0b: break;
        case 0x0c: return get_subclass_0xc_string(pci_subclass, pinterface);
        case 0xff: return "MISC";
    }
    return "";
}

static inline char* get_cap_string(uint8_t cap_id){
    switch(cap_id){
        case 0x01: return "Power Management";
        case 0x02: return "Accelerated Graphics Port";
        case 0x03: return "Vital Product Data";
        case 0x04: return "Slot Identification";
        case 0x05: return "Message Signalled Interrupts";
        case 0x06: return "CompactPCI HotSwa";
        case 0x07: return "PCI-X";
        case 0x08: return "HyperTransport";
        case 0x09: return "Vendor-Specific";
        case 0x0a: return "Debug port";
        case 0x0b: return "CompactPCI Central Resource Control";
        case 0x0c: return "PCI Standard Hot-Plug Controller";
        case 0x0d: return "Bridge subsystem vendor/device ID";
        case 0x0e: return "AGP Target PCI-PCI bridge";
        case 0x0f: return "Secure Device";
        case 0x10: return "PCI Express";
        case 0x11: return "MSI-X";
        case 0x12: return "SATA Data/Index Conf.";
        case 0x13: return "PCI Advanced Features";
    }
    return "";
}

static inline uint32_t pci_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    return (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
}

static inline uintptr_t pci_ecam_address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    return (pci_ecam_ptr + (bus << 20 | slot << 15 | func << 12) + (offset & 0xfc));
}

static uint8_t pci_read_8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, bool ecam){
    if(ecam){
        return (uint8_t)(read_physmem_32((pci_ecam_address(bus, slot, func, offset))) >> ((offset%4) * 8)) & 0xff;
    }
    outl(pci_address(bus, slot, func, offset), PCI_IO_CONFIG_ADDRESS);
    return (uint8_t)((inl(PCI_IO_CONFIG_DATA) >> ((offset%4) * 8)) & 0xff);
}

static uint16_t pci_read_16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, bool ecam){
    if(ecam){
        return (uint16_t)(read_physmem_32((pci_ecam_address(bus, slot, func, offset))) >> (offset & 2) * 8) & 0xffff;
    }
	outl(pci_address(bus, slot, func, offset), PCI_IO_CONFIG_ADDRESS);
	return (uint16_t)((inl(PCI_IO_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
}

static uint32_t pci_read_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, bool ecam){
    if(ecam){
        return read_physmem_32(pci_ecam_address(bus, slot, func, offset));
    }
    outl(pci_address(bus, slot, func, offset), PCI_IO_CONFIG_ADDRESS);
    return (uint32_t)inl(PCI_IO_CONFIG_DATA);
}

static void pci_write_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data, bool ecam){
    if(ecam){
        write_physmem_32(pci_ecam_address(bus, slot, func, offset), data);
        return;
    }
    outl(pci_address(bus, slot, func, offset), PCI_IO_CONFIG_ADDRESS);
    outl(data, PCI_IO_CONFIG_DATA);
}

static uintptr_t pcie_get_ptr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    return (uintptr_t) pci_ecam_address(bus, slot, func, offset);
}  

static void pci_msi_setup(struct msi_cap* data){
	//uint32_t address = apic_get_base() | (apic_get_lapic_id() << 12);
	uint32_t address = apic_get_base() | (apic_get_local_id() << 12); /* lapicid = 0 */
    uint8_t vector = 66+alloc_msi_vectors(1);

    address = 0x00025000;

    switch((read_physmem_16((uintptr_t)&(data->message_control)) >> 7) & 0x3){

    	case 0b00: 	printf(STR_PREFIX"        Type 00 (vector: %1x)\n\r", vector-56); 
                    write_physmem_32((uintptr_t)&(((struct msi_cap32*)data)->message_address), address);
                    write_physmem_16((uintptr_t)&(((struct msi_cap32*)data)->message_data), vector);
                    /* enable */
                    write_physmem_16((uintptr_t)&(((struct msi_cap32*)data)->message_control), read_physmem_16((uintptr_t)&(((struct msi_cap32*)data)->message_control)) | 0x1);
					break;
		case 0b01:	printf(STR_PREFIX"        Type 01 (vector: %1x)\n\r", vector-56);
                    write_physmem_32((uintptr_t)&(((struct msi_cap64*)data)->message_address), address);
                    write_physmem_32((uintptr_t)&(((struct msi_cap64*)data)->message_address_upper), 0x0);
                    write_physmem_16((uintptr_t)&(((struct msi_cap64*)data)->message_data), vector);
                    /* enable */
                    write_physmem_16((uintptr_t)&(((struct msi_cap64*)data)->message_control), read_physmem_16((uintptr_t)&(((struct msi_cap64*)data)->message_control)) | 0x1);
					break;
		case 0b10: 	printf(STR_PREFIX"        Type 10 (vector: %1x)\n\r", vector-56); 
                    write_physmem_32((uintptr_t)&(((struct msi_cap32_vec*)data)->message_address), address);
                    write_physmem_16((uintptr_t)&(((struct msi_cap32_vec*)data)->message_data), vector);
                    /* enable */
                    write_physmem_16((uintptr_t)&(((struct msi_cap32_vec*)data)->message_control), read_physmem_16((uintptr_t)&(((struct msi_cap32_vec*)data)->message_control)) | 0x1);
					break;
		case 0b11: 	printf(STR_PREFIX"        Type 11 (vector: %1x)\n\r", vector-56); 
                    write_physmem_32((uintptr_t)&(((struct msi_cap64_vec*)data)->message_address), address);
                    write_physmem_32((uintptr_t)&(((struct msi_cap64_vec*)data)->message_address_upper), 0x0);
                    write_physmem_16((uintptr_t)&(((struct msi_cap64_vec*)data)->message_data), vector);
                    /* enable */
                    write_physmem_16((uintptr_t)&(((struct msi_cap64_vec*)data)->message_control), read_physmem_16((uintptr_t)&(((struct msi_cap64_vec*)data)->message_control)) | 0x1);
					break;
	}
}

static void pci_parse_msi_cap(uintptr_t cap){
    struct msi_cap* data = (struct msi_cap*) cap;
    
    printf(STR_PREFIX"      cap-id: %1x\n\r", data->cap_id);
    printf(STR_PREFIX"      next_cap: %1x\n\r", data->next_cap);
    printf(STR_PREFIX"      message_control: %2x\n\r", data->message_control);

    printf(STR_PREFIX"      -> per-vector masking:     %1x\n\r", !!(data->message_control & 0x100));
    printf(STR_PREFIX"      -> 64-Bit capable:         %1x\n\r", !!(data->message_control & 0x80));
    printf(STR_PREFIX"      -> per-vector masking:     %1x\n\r", (data->message_control & 0x100));

    uint8_t vectors = 1 << ((data->message_control >> 4) & 0x7);
    if(vectors > 32) {
    	vectors = 0xff;
   	
   	}
    printf(STR_PREFIX"      -> multi-message enabled:  %d\n\r", vectors);

    vectors = 1 << ((data->message_control >> 1) & 0x7);
    if(vectors > 32) {
    	vectors = 0xff;
   	
   	}
    printf(STR_PREFIX"      -> multi-message capable:  %d\n\r", vectors);
    printf(STR_PREFIX"      -> enabled:                %1x\n\r", !!(data->message_control & 0x1));
    
    pci_msi_setup(data);
}

static void pci_parse_pcie_cap(uintptr_t cap){
    struct pci_express_cap* data = (struct pci_express_cap*) cap;

    printf(STR_PREFIX"      cap-id: %1x\n\r", read_physmem_8((uintptr_t)&(data->pcie_cap_id)));
    printf(STR_PREFIX"      next_cap: %1x\n\r", read_physmem_8((uintptr_t)&(data->next_cap)));
    printf(STR_PREFIX"      pcie_cap_register: %2x\n\r", read_physmem_16((uintptr_t)&(data->pcie_cap_register)));
    printf(STR_PREFIX"      device_cap: %x\n\r", read_physmem_32((uintptr_t)&(data->device_cap)));
    printf(STR_PREFIX"      device_control: %2x\n\r", read_physmem_16((uintptr_t)&(data->device_control)));
    printf(STR_PREFIX"      device_status: %2x\n\r", read_physmem_16((uintptr_t)&(data->device_status)));
}


static uint32_t enumerate_pci_erom_bar(uint8_t bus, uint8_t slot, uint8_t function, bool ecam){
    uint32_t bar;
    bar = pci_read_32(bus, slot, function, PCI_CONF_EXP_ROM_32, ecam);
    if(bar){
        /* if disabled, enable the expension rom base address */
        if (!(bar & 0x1)){ 
            pci_write_32(bus, slot, function, PCI_CONF_EXP_ROM_32, bar+1, ecam);
        }
        if(pci_read_32(bus, slot, function, PCI_CONF_EXP_ROM_32, ecam) & 0x1){
            return bar;
        }
    }
    return 0x0;
}

static uint8_t global_bus = 0;


static void enumerate_pci_device(pci_state_t* self, uint8_t bus, uint8_t slot, uint8_t function, bool ecam){
    bool bridge = false;
    uint8_t class, subclass, pinterface, cptr;
    uint16_t vendor, device;
    uint32_t bar, size;
    uint8_t pci_bars = 6;
    if ((vendor = pci_read_16(bus, slot, function, PCI_CONF_VENDOR_ID_16, ecam)) != PCI_NO_DEVICE) {
        self->devices[self->num_devices] = (pci_device_t*) kmalloc(sizeof(pci_device_t));

        device = pci_read_16(bus, slot, function, PCI_CONF_DEVICE_ID_16, ecam);
        printf(STR_PREFIX" Bus:%2x Slot:%2x Func:%2x %4x:%4x \n\r", bus, slot, function, vendor, device); 
        printf(STR_PREFIX" -------------------------------\n\r");


        self->devices[self->num_devices]->bus = bus;
        self->devices[self->num_devices]->slot = slot;
        self->devices[self->num_devices]->func = function;


        self->devices[self->num_devices]->vendor = vendor;
        self->devices[self->num_devices]->product = device;

        class =         pci_read_8(bus, slot, function, PCI_CONF_CLASS_8, ecam);
        subclass =      pci_read_8(bus, slot, function, PCI_CONF_SUBCLASS_8, ecam);
        pinterface =    pci_read_8(bus, slot, function, PCI_CONF_PINTERF_8, ecam);
        cptr =          pci_read_8(bus, slot, function, PCI_CONF_CPTR_8, ecam) & 0xfc;

        self->devices[self->num_devices]->class = class;
        self->devices[self->num_devices]->subclass = subclass;
        self->devices[self->num_devices]->interface = pinterface;
        self->devices[self->num_devices]->class = class;


        printf(STR_PREFIX"   %2x:%2x:%2x (%s, %s)  CP: %2x\n\r", class, subclass, pinterface, get_class_string(class), get_subclass_string(class, subclass, pinterface), cptr);

         self->devices[self->num_devices]->bars = (pci_bar_t**) kmalloc(sizeof(pci_bar_t*) * pci_bars);
        self->devices[self->num_devices]->num_bars = 0;
        if (class == 0x6 && subclass == 0x4){
            if(ecam){
            self->devices[self->num_devices]->configuration_space_addr = (uint32_t) pci_ecam_address(bus, slot, function, 0);
        }else{
            self->devices[self->num_devices]->configuration_space_addr = (uint32_t) pci_address(bus, slot, function, 0);
        }
        self->devices[self->num_devices]->ecam = ecam;

            self->num_devices++;   
            return;
            pci_bars = 2;
            bridge = true;
        }        


        /* Enable BUS MASTERING */
        uint16_t command_value_16 = pci_read_16(bus, slot, function, PCI_CONF_COMMAND_16, ecam);

        uint32_t command_value = pci_read_32(bus, slot, function, PCI_CONF_COMMAND_16, ecam);

        if(!(command_value & 0x4)){
            pci_write_32(bus, slot, function, PCI_CONF_COMMAND_16, command_value | 0x4, ecam);
            command_value_16 = pci_read_16(bus, slot, function, PCI_CONF_COMMAND_16, ecam);
            if(!(command_value_16 & 0x4)){
                printf(STR_PREFIX"   BM DMA:\t *failed*\n");
            }
            else{
                printf(STR_PREFIX"   BM DMA:\t *enabled*\n");
            }
        }
        else{
            printf(STR_PREFIX"   BM DMA:\t *enabled by default*\n");
        }

        uint8_t offset = 0;
        for(uint8_t i = 0; i < pci_bars; i++) {

            bar = pci_read_32(bus, slot, function, PCI_CONF_BAR_0_32 + (4*i), ecam);
            pci_write_32(bus, slot, function, PCI_CONF_BAR_0_32 + (4*i), 0xfffffff0, ecam);
            size = (~(pci_read_32(bus, slot, function, PCI_CONF_BAR_0_32 + (4*i), ecam) & 0xfffffff0))+1;
            pci_write_32(bus, slot, function, PCI_CONF_BAR_0_32 + (4*i), bar, ecam);
            if(bar){

                self->devices[self->num_devices]->bars[offset] = (pci_bar_t*) kmalloc(sizeof(pci_bar_t));
                self->devices[self->num_devices]->bars[offset]->address = bar&0xfffffff0;
                self->devices[self->num_devices]->bars[offset]->size = size;
                self->devices[self->num_devices]->bars[offset]->type = (uint8_t) bar&0xf; 
                self->devices[self->num_devices]->bars[offset]->int_pin = pci_read_8(bus, slot, function, PCI_CONF_INT_PIN_8, ecam); 
                self->devices[self->num_devices]->bars[offset]->int_line = pci_read_8(bus, slot, function, PCI_CONF_INT_LINE_8, ecam); 

                offset++;

                if (bar&0x4){
                    /* fix for 64 bit BAR size? */
                    printf(STR_PREFIX"   BAR%d: 0x%x%x [0x%x]", i, pci_read_32(bus, slot, function, PCI_CONF_BAR_0_32 + (4*(i+1)), ecam) &0xfffffff0, bar &0xfffffff0, size);
                } 
                else {
                    printf(STR_PREFIX"   BAR%d:         0x%8x [0x%x]", i, bar &0xfffffff0, size);
                }

                printf (" IRQ %d:%d ", pci_read_8(bus, slot, function, PCI_CONF_INT_PIN_8, ecam), pci_read_8(bus, slot, function, PCI_CONF_INT_LINE_8, ecam));

                if (bar&0x1){
                    /* PCI IO Relocation */
                    printf(" (I/O)");    
                }

                if (bar&0x2){
                    printf(" (< 1MiB)");
                }

                if (bar&0x4){
                    printf(" (64 Bit)");
                    i++;
                }

                if (bar&0x8){
                    printf(" (prefetchable)");
                }

                printf("\n\r");

                self->devices[self->num_devices]->num_bars++;
            }
        }

        bar = enumerate_pci_erom_bar(bus, slot, function, ecam);
        if(bar){
            printf(STR_PREFIX"   BAR6:         0x%8x [          ] (ROM)\n\r", bar & 0xfffffff0);
        }

        if(ecam){
            self->devices[self->num_devices]->configuration_space_addr = (uint32_t) pci_ecam_address(bus, slot, function, 0);
        }else{
            self->devices[self->num_devices]->configuration_space_addr = (uint32_t) pci_address(bus, slot, function, 0);
        }
        self->devices[self->num_devices]->ecam = ecam;


        /* PCI - ISA Bridge (LPC) */
        if((class == 0x6 && subclass == 0x01 && pinterface == 0x00) && ecam){
            bar = (read_physmem_32(self->devices[self->num_devices]->configuration_space_addr + 0xf0)& 0xfffffff0);

            self->devices[self->num_devices]->bars[offset] = (pci_bar_t*) kmalloc(sizeof(pci_bar_t));
            self->devices[self->num_devices]->bars[offset]->address = bar&0xfffffff0;
            self->devices[self->num_devices]->bars[offset]->size = 0x4000;
            self->devices[self->num_devices]->bars[offset]->type = (uint8_t) bar&0xf; 
            self->devices[self->num_devices]->bars[offset]->int_pin = 0; 
            self->devices[self->num_devices]->bars[offset]->int_line = 0; 
            self->devices[self->num_devices]->num_bars++;
        }
        

        uint8_t cptr_id;
        uint8_t cptr_next;
        uint8_t cptr_count = 0;
        while(0){
            if (!(cptr)){
                break;
            }
            cptr_id = pci_read_8(bus, slot, function, cptr, ecam); 
            cptr_next = pci_read_8(bus, slot, function, cptr+1, ecam) & 0xfc; 
            printf(STR_PREFIX"   CAP%d: 0x%2x %s\n\r", cptr_count++, cptr, get_cap_string(cptr_id));
            //printf("    cptr: %2x id: %2x next: %2x (%s)\n\r", cptr, cptr_id, cptr_next, get_cap_string(cptr_id));

            uintptr_t cap = (uintptr_t) pcie_get_ptr(bus, slot, function, cptr);
            
            switch(cptr_id){ 
                case 0x05:
                    pci_parse_msi_cap(cap);
                    break;

                
                case 0x10: // PCI Express Cap
                    pci_parse_pcie_cap(cap);
                    break;
                }
            
            cptr = cptr_next;
        }
        printf(STR_PREFIX"\n\r");
     
        self->num_devices++;   
    }

    if(bridge){
        printf("BRIDGE\n\r");
        global_bus++;
        pci_bus_enumeration(self, global_bus, ecam);
    }

    return ;
 }

static void checkDevice(pci_state_t* self, uint8_t bus, uint8_t device, bool ecam) {
    for(uint8_t func = 0; func < 8; func++){
    	enumerate_pci_device(self, bus, device, func, ecam);
    }
}

static void pci_enumeration_bus(pci_state_t* self, uint16_t bus, bool ecam){
    for(uint8_t device = 0; device < PCI_MAX_SLOT; device++) {
        checkDevice(self, bus, device, ecam);
    }
}

static void pci_bus_enumeration(pci_state_t* self, uint8_t bus, bool ecam){

    for(uint16_t bus = 0; bus < 8; bus++) {
         for(uint16_t device = 0; device < 32; device++) {
             checkDevice(self, bus, device, ecam);
         }
     }
     return;

    if(!(pci_read_8(bus, 0, 0, PCI_CONF_HEADER_TYPE_8, ecam) & 0x80)){
        /* Single PCI host controller */
        pci_enumeration_bus(self, bus, ecam);

    } else {
        /* Multiple PCI host controllers */
        for(uint8_t function = 0; function < 8; function++) {
            if(pci_read_16(bus, 0, function, PCI_CONF_VENDOR_ID_16, ecam) != PCI_NO_DEVICE){
                pci_enumeration_bus(self, function, ecam);
            }
        }
    }
}

void set_pci_ecam_address(uintptr_t address){
    printf("-------------------------------->>>>>\n\r");
    pci_ecam_ptr = address;
}

pci_state_t* pci_state_new(void){
    pci_state_t* self = (pci_state_t*)kmalloc(sizeof(pci_state_t));
    self->num_devices = 0;
    return self;
}

/* Todo: Legacy PCI 1.0 Enumeration http://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_.232 */

void pci_state_enum(pci_state_t* self){
    global_bus = 0;
    pci_bus_enumeration(self, 0, (bool)(!!(pci_ecam_ptr)));
    return;

    printf("--------------DEVICES: %d\n\r", self->num_devices);
    for (uint16_t i = 0; i < self->num_devices; i++){
        printf("(%d) Bus:%2x Slot:%2x Func:%2x %4x:%4x (%2x %2x %2x) bars: %d\n\r", i,  
                                                            self->devices[i]->bus,  
                                                            self->devices[i]->slot,  
                                                            self->devices[i]->func,  
                                                            self->devices[i]->vendor,  
                                                            self->devices[i]->product,
                                                            self->devices[i]->class,
                                                            self->devices[i]->subclass,
                                                            self->devices[i]->interface,
                                                            self->devices[i]->num_bars);
        for(uint16_t j = 0; j < self->devices[i]->num_bars; j++){
            printf("    BAR: 0x%x-0x%x | %x | %d:%d\n\r",  
                                        self->devices[i]->bars[j]->address, 
                                        self->devices[i]->bars[j]->address+self->devices[i]->bars[j]->size, 
                                        self->devices[i]->bars[j]->type, 
                                        self->devices[i]->bars[j]->int_pin, 
                                        self->devices[i]->bars[j]->int_line);

        } 

        if(self->devices[i]->ecam){
            printf("    ECAM: 0x%x-0x%x\n\r", self->devices[i]->configuration_space_addr, self->devices[i]->configuration_space_addr + 0x1000);
        }
        else{
            printf("    LEGACY: 0x%x-0x%x\n\r", self->devices[i]->configuration_space_addr, self->devices[i]->configuration_space_addr + 0x100);
        }
    }
}

void pci_register_areas(pci_state_t* self, fuzzer_state_t* fuzzer){

#ifdef EXTENDED_PCI_FUZZING
    if(pci_ecam_ptr){
        register_area(fuzzer, pci_ecam_ptr, 0x20000, 1, "ECAM");
    }

    register_area(fuzzer, pci_ecam_ptr, 0xcf8, 4, "IO PCI CFX");
#endif

    for (uint16_t i = 0; i < self->num_devices; i++){
        
	    for(uint16_t j = 0; j < self->devices[i]->num_bars; j++){
            register_area(fuzzer, self->devices[i]->bars[j]->address, self->devices[i]->bars[j]->size, !(self->devices[i]->bars[j]->type&0x1), get_subclass_string(self->devices[i]->class, self->devices[i]->subclass, self->devices[i]->interface));
        }
	
#ifdef EXTENDED_PCI_FUZZING        
        if(self->devices[i]->ecam){
            register_area(fuzzer, self->devices[i]->configuration_space_addr, 128, 1, get_subclass_string(self->devices[i]->class, self->devices[i]->subclass, self->devices[i]->interface));
        }
        else{
            register_area(fuzzer, self->devices[i]->configuration_space_addr, 128, 0, get_subclass_string(self->devices[i]->class, self->devices[i]->subclass, self->devices[i]->interface));
        }
#endif     

        
        
    }
}
