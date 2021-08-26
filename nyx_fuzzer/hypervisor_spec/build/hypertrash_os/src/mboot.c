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

#include "mboot.h"
#include "mbootv1.h"

#include "tty.h"
#include "efi.h"
#include "acpi.h"
#include "mem.h"

#define STR_PREFIX	" [MBOOT]"

static char* e820_types[] = {
	"",
	"Usable",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"Bad",
};

static char* efi_mem_types[] = {
	"Reserved",
	"Loader Code",
	"Loader Data",
	"Boot Sevice Code",
	"Boot Sevice Data",
	"Runtime Service Code",
	"Runtime Service Data",
	"Usable",
	"Unusable",
	"ACPI Reclaimable",
	"ACPI NVS",
	"MMIO",
	"MMIO Port Space",
	"PAL Code",
	"Persistent Memory",
	"Max Memory Type",
};

uintptr_t efi_est = (uintptr_t)NULL; 

static void parse_efi_mmap(uintptr_t data){
	struct multiboot_tag_efi_mmap* mbi = (struct multiboot_tag_efi_mmap*) data;
	uint32_t entries = (mbi->size-16)/sizeof(efi_memory_desc_t);
	printf (STR_PREFIX"   descr_size: %x [entries: %x]\n\r", mbi->descr_size, entries);
	printf (STR_PREFIX"   descr_vers: %x\n\r", mbi->descr_vers);
	printf (STR_PREFIX"   entries0: %x\n\r", mbi->efi_mmap);

	efi_memory_desc_t* tmp = (efi_memory_desc_t*)mbi->efi_mmap;
	for(uint64_t i = 0; i < entries; i++){
		printf(STR_PREFIX" EFI: 0x%x-0x%x [%s]\n\r", tmp[i].phys_addr_a, (tmp[i].phys_addr_a+(tmp[i].num_pages_a << EFI_PAGE_SHIFT))-1, efi_mem_types[tmp[i].type > 15 ? 0 : tmp[i].type]);
		switch (tmp[i].type){
			case 1: /* loader code */
			case 2: /* loader data */
			case 3:	/* boot service code */
			case 4:	/* boot service data */
				tmp[i].type = 7; /* usable */
				break;
		}
	}
	//mem_set_efi_mmap((uintptr_t) mbi->efi_mmap);
}

static void parse_mmap(uintptr_t data){
	struct multiboot_tag_mmap* mbi = (struct multiboot_tag_mmap*) data;
	uint32_t entries = (mbi->size-16)/sizeof(struct multiboot_mmap_entry);
	printf (STR_PREFIX"   entry_size: %x [entries: %x]\n\r", mbi->entry_size, entries);
	printf (STR_PREFIX"   entry_version: %x\n\r", mbi->entry_version);
	printf (STR_PREFIX"   entries0: %x\n\r", mbi->entries);


	for(uint32_t i = 0; i < entries; i++){
		printf (STR_PREFIX" E820: 0x%x-0x%x [%s]\n\r", mbi->entries[i].addr_a, (mbi->entries[i].addr_a + mbi->entries[i].len_a-1), e820_types[mbi->entries[i].type > 5 ? 2 : mbi->entries[i].type]);
	}
	mem_set_e820_mmap((uintptr_t) mbi->entries, entries);
}

static void parse_efi32(uintptr_t data){
	struct multiboot_tag_efi32* mbi = (struct multiboot_tag_efi32*) data;
	printf (STR_PREFIX"   efi32: %x\n\r", mbi->pointer);
	efi_est = mbi->pointer;
}

static void parse_cmdline(uintptr_t data){
	struct multiboot_tag_string* mbi = (struct multiboot_tag_string*) data;
	printf (STR_PREFIX"   cmdline: %s\n\r", mbi->string);
}

static void parse_bootloader_name(uintptr_t data){
	struct multiboot_tag_string* mbi = (struct multiboot_tag_string*) data;
	printf (STR_PREFIX"   bootloader: %s\n\r", mbi->string);
}

static void parse_bootdev(uintptr_t data){
	struct multiboot_tag_bootdev* mbi = (struct multiboot_tag_bootdev*) data;
	printf (STR_PREFIX"   bootdev: 0x%x,%x,%x\n\r", mbi->biosdev, mbi->slice, mbi->part);
}

static void parse_basic_meminfo(uintptr_t data){
	struct multiboot_tag_basic_meminfo* mbi = (struct multiboot_tag_basic_meminfo*) data;
	printf(STR_PREFIX"   mem_lower: 0x%x KB\n\r", mbi->mem_lower);
	printf(STR_PREFIX"   mem_upper: 0x%x KB\n\r", mbi->mem_upper);
	mem_set_low_high(mbi->mem_lower, mbi->mem_upper);
	printf(STR_PREFIX"   mem_total: 0x%x KB\n\r", (mbi->mem_lower + mbi->mem_upper));
}

static void parse_basic_old_acpi(uintptr_t data){
	struct multiboot_tag_old_acpi* mbi = (struct multiboot_tag_old_acpi*) data;
	printf(STR_PREFIX"   acpi rsdp: 0x%x (new)\n\r", mbi->rsdp);
	apci_set_rsdp_addr((uintptr_t)mbi->rsdp);
}

static void parse_basic_new_acpi(uintptr_t data){
	struct multiboot_tag_new_acpi* mbi = (struct multiboot_tag_new_acpi*) data;
	printf(STR_PREFIX"   acpi rsdp: 0x%x (new)\n\r", mbi->rsdp);
	apci_set_rsdp_addr((uintptr_t)mbi->rsdp);
}

char* mboot_tag[] = {
	"END",
	"CMDLINE",
	"BOOT_LOADER_NAME",
	"MODULE",
	"BASIC_MEMINFO",
	"BOOTDEV",
	"MMAP",
	"VBE",
	"FRAMEBUFFER",
	"ELF_SECTIONS",
	"APM",
	"EFI32",
	"EFI64",
	"SMBIOS",
	"ACPI_OLD",
	"ACPI_NEW",
	"NETWORK",
	"EFI_MMAP",
	"EFI_BS",
	"EFI32_IH",
	"EFI64_IH",
	"LOAD_BASE_ADDR",
};

static void parse_basic_meminfo_v1(struct multiboot *mboot){
	printf(STR_PREFIX"  v1 mem_lower: 0x%x KB\n\r", mboot->mem_lower);
	printf(STR_PREFIX"  v1 mem_upper: 0x%x KB\n\r", mboot->mem_upper);
	mem_set_low_high(mboot->mem_lower, mboot->mem_upper);
	printf(STR_PREFIX"  v1 mem_total: 0x%x KB\n\r", (mboot->mem_lower + mboot->mem_upper));
}


static bool parse_mmap_v1(struct multiboot *mboot){

	if(mboot->mmap_addr) {
		mboot_memmap_t* mmap = (void*)mboot->mmap_addr;

		while((uintptr_t)mmap < mboot->mmap_addr + mboot->mmap_length) {
			printf (STR_PREFIX" E820: 0x%x-0x%x [%s]\n\r", mmap->base_addr_a , (mmap->base_addr_a+mmap->length_a-1), e820_types[mmap->type > 5 ? 2 : mmap->type]);

			mmap = (mboot_memmap_t *) ((uintptr_t)mmap + mmap->size + sizeof(uintptr_t));
		}
	}

	return true;
}

bool mboot_init_v1(struct multiboot *mboot){

	parse_basic_meminfo_v1(mboot);
	return parse_mmap_v1(mboot);

}

bool mboot_init(struct multiboot_tag* mbi, uint32_t mboot_magic){
	uint32_t size;
	struct multiboot_tag *tag;


	if (mboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC){

		if(mboot_magic == MULTIBOOT_MAGIC || mboot_magic == MULTIBOOT_EAX_MAGIC){
			printf(STR_PREFIX" Multiboot version 1 bootloader detected... (ptr: %x)\n\r", (unsigned) mbi);
			return mboot_init_v1((struct multiboot *)mbi);
		}

		printf(STR_PREFIX" invalid magic number: 0x%x\n\r", (unsigned) mboot_magic);
		return false;
	}

	if (((uintptr_t)mbi) & 7){
		printf(STR_PREFIX" unaligned mbi: 0x%x\n\r", mbi);
		return false;
    }

    size = *((uint32_t*) mbi);
    printf(STR_PREFIX" multiboot2 mbi size: 0x%x\n\r", size);


    for (tag = (struct multiboot_tag *) (mbi + 8); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))){
    	printf(STR_PREFIX" %s [0x%2x]\n\r", mboot_tag[tag->type], tag->size);
    	switch (tag->type){
			case MULTIBOOT_TAG_TYPE_CMDLINE: 					parse_cmdline((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:	parse_bootloader_name((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_MODULE: 					break;
			case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: 		parse_basic_meminfo((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_BOOTDEV: 					parse_bootdev((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_MMAP: 						parse_mmap((uintptr_t)tag); break;;
			case MULTIBOOT_TAG_TYPE_VBE: 							break;
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: 			break;
			case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: 		break;
			case MULTIBOOT_TAG_TYPE_APM: 							break;
			case MULTIBOOT_TAG_TYPE_EFI32: 						parse_efi32((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_EFI64: 						break;
			case MULTIBOOT_TAG_TYPE_SMBIOS: 					break;
			case MULTIBOOT_TAG_TYPE_ACPI_OLD: 				parse_basic_old_acpi((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_ACPI_NEW: 				parse_basic_new_acpi((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_NETWORK: 					break;
			case MULTIBOOT_TAG_TYPE_EFI_MMAP: 				parse_efi_mmap((uintptr_t)tag); break;
			case MULTIBOOT_TAG_TYPE_EFI_BS: 					break;
			case MULTIBOOT_TAG_TYPE_EFI32_IH: 				break;
			case MULTIBOOT_TAG_TYPE_EFI64_IH: 				break;
			case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: 	break;
    	}
    }

	if (efi_est){
		efi_parse_est32(efi_est);
	}

	return true;
}