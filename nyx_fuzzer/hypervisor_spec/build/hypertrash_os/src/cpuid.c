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

#include "cpuid.h"
#include "tty.h"
#include "msr.h"

#define STR_PREFIX	" [CPUID]"

bool supports_vmx(void){
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);

	//printf("Checking VMX...(%x)\n\r", (CHECK(ecx, f_ecx_bit_VMX)));
	return  (CHECK(ecx, f_ecx_bit_VMX));

	//return 1;
}

static void print_cpu_features(void){
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);
	printf(STR_PREFIX" features: ");
	if (CHECK(edx, f_edx_bit_FPU))			printf("FPU ");
	if (CHECK(edx, f_edx_bit_VME))			printf("VME ");
	if (CHECK(edx, f_edx_bit_DE)) 			printf("DE ");
	if (CHECK(edx, f_edx_bit_PSE))			printf("PSE ");
	if (CHECK(edx, f_edx_bit_TSC))			printf("TSC ");
	if (CHECK(edx, f_edx_bit_MSR))			printf("MSR ");
	if (CHECK(edx, f_edx_bit_PAE))			printf("PAE ");
	if (CHECK(edx, f_edx_bit_MCE))			printf("MCE ");
	if (CHECK(edx, f_edx_bit_CX8))			printf("CX8 ");
	if (CHECK(edx, f_edx_bit_APIC))			printf("APIC ");
	if (CHECK(edx, f_edx_bit_SEP))			printf("SEP ");
	if (CHECK(edx, f_edx_bit_MTRR))			printf("MTRR ");
	if (CHECK(edx, f_edx_bit_PGE))			printf("PEG ");
	if (CHECK(edx, f_edx_bit_MCA))			printf("MCA ");
	if (CHECK(edx, f_edx_bit_CMOV))	 		printf("CMOV ");
	if (CHECK(edx, f_edx_bit_CMOV))			printf("PSE36 ");
	if (CHECK(edx, f_edx_bit_PSN))			printf("PSN ");
	if (CHECK(edx, f_edx_bit_CLFSH))		printf("CLFSH ");
	if (CHECK(edx, f_edx_bit_DS))				printf("DS ");
	if (CHECK(edx, f_edx_bit_ACPI))			printf("ACPI ");
	if (CHECK(edx, f_edx_bit_MMX))			printf("MMX ");
	if (CHECK(edx, f_edx_bit_FXSR))			printf("FXSR ");
	if (CHECK(edx, f_edx_bit_SSE))			printf("SSE ");
	if (CHECK(edx, f_edx_bit_SSE2))			printf("SSE2 ");
	if (CHECK(edx, f_edx_bit_SS))				printf("SS ");
	if (CHECK(edx, f_edx_bit_HTT))	 		printf("HTT ");
	if (CHECK(edx, f_edx_bit_TM))		  	printf("TM ");
	if (CHECK(edx, f_edx_bit_IA64))	  	printf("IA64 ");
	if (CHECK(edx, f_edx_bit_PBE))	 		printf("PBE ");
	if (CHECK(ecx, f_ecx_bit_SSE3))	 		printf("SSE3 ");
	if (CHECK(ecx, f_ecx_bit_PCLMULQDQ))	printf("PCLMULQDQ ");
	if (CHECK(ecx, f_ecx_bit_DTES64))	 	printf("DTES64 ");
	if (CHECK(ecx, f_ecx_bit_MONITOR)) 	printf("MONITOR ");
	if (CHECK(ecx, f_ecx_bit_DS_CPL))	 	printf("DS_CPL ");
	if (CHECK(ecx, f_ecx_bit_VMX))	 		printf("VMX ");
	if (CHECK(ecx, f_ecx_bit_SMX))	 		printf("SMX ");
	if (CHECK(ecx, f_ecx_bit_EST))	 		printf("EST ");
	if (CHECK(ecx, f_ecx_bit_TM2))	 		printf("TM2 ");
	if (CHECK(ecx, f_ecx_bit_SSSE3))	 	printf("SSSE3 ");
	if (CHECK(ecx, f_ecx_bit_CNXT_ID))	printf("CNXT_ID ");
	if (CHECK(ecx, f_ecx_bit_SDBG))	 		printf("SDBG ");
	if (CHECK(ecx, f_ecx_bit_FMA))	 		printf("FMA ");
	if (CHECK(ecx, f_ecx_bit_CX16))	 		printf("CX16 ");
	if (CHECK(ecx, f_ecx_bit_XTPR))	 		printf("XTPR ");
	if (CHECK(ecx, f_ecx_bit_PDCM))	 		printf("PDCM ");
	if (CHECK(ecx, f_ecx_bit_PCID))	 		printf("PCID ");
	if (CHECK(ecx, f_ecx_bit_DCA))	 		printf("DCA ");
	if (CHECK(ecx, f_ecx_bit_SSE41))	 	printf("SSE41 ");
	if (CHECK(ecx, f_ecx_bit_SSE42))	 	printf("SSE42 ");
	if (CHECK(ecx, f_ecx_bit_X2APIC))	 	printf("X2APIC ");
	if (CHECK(ecx, f_ecx_bit_MOVBE))	 	printf("MOVBE ");
	if (CHECK(ecx, f_ecx_bit_POPCNT))	 	printf("POPCNT ");
	if (CHECK(ecx, f_ecx_bit_TSC_DL))	 	printf("TSC_DL ");
	if (CHECK(ecx, f_ecx_bit_AES))	 		printf("AES ");
	if (CHECK(ecx, f_ecx_bit_XSAVE))	 	printf("XSAVE ");
	if (CHECK(ecx, f_ecx_bit_OXSAVE))	 	printf("OXSAVE ");
	if (CHECK(ecx, f_ecx_bit_AVX))	 		printf("AVX ");
	if (CHECK(ecx, f_ecx_bit_F16C))	 		printf("F16C ");
	if (CHECK(ecx, f_ecx_bit_RDRND))	 	printf("RDRND ");
	if (CHECK(ecx, f_ecx_bit_HYPERVISOR))	printf("HYPERVISOR ");
	printf("\n\r");
}

static void print_extended_cpu_features(void){
	uint32_t eax = 7;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);
	printf(STR_PREFIX" extended features: ");
	if (CHECK(ebx, ef_ebx_bit_FSGSBASE))		printf("FSGSBASE ");
	if (CHECK(ebx, ef_ebx_bit_TSC_ADJUST))		printf("TSC_ADJUST ");
	if (CHECK(ebx, ef_ebx_bit_SGX))				printf("SGX ");
	if (CHECK(ebx, ef_ebx_bit_BM1))				printf("BM1 ");
	if (CHECK(ebx, ef_ebx_bit_HLE))				printf("HLE ");
	if (CHECK(ebx, ef_ebx_bit_AVX))				printf("AVX ");
	if (CHECK(ebx, ef_ebx_bit_SMEP))			printf("SMEP ");
	if (CHECK(ebx, ef_ebx_bit_BMI2))			printf("BMI2 ");
	if (CHECK(ebx, ef_ebx_bit_ERMS))			printf("ERMS ");
	if (CHECK(ebx, ef_ebx_bit_INVPCID))			printf("INVPCID ");
	if (CHECK(ebx, ef_ebx_bit_RTM))				printf("RTM ");
	if (CHECK(ebx, ef_ebx_bit_PQM))				printf("PQM ");
	if (CHECK(ebx, ef_ebx_bit_MPX))				printf("MPX ");
	if (CHECK(ebx, ef_ebx_bit_PQE))				printf("PQE ");
	if (CHECK(ebx, ef_ebx_bit_AVX512F))			printf("AVX512F ");
	if (CHECK(ebx, ef_ebx_bit_AVX512DQ))		printf("AVX512DQ ");
	if (CHECK(ebx, ef_ebx_bit_RDSEED))			printf("RDSEED ");
	if (CHECK(ebx, ef_ebx_bit_ADX))				printf("ADX ");
	if (CHECK(ebx, ef_ebx_bit_SMAP))			printf("SMAP ");
	if (CHECK(ebx, ef_ebx_bit_AVX512IFMA))		printf("AVX512IFMA ");
	if (CHECK(ebx, ef_ebx_bit_PCOMMIT))			printf("PCOMMIT ");
	if (CHECK(ebx, ef_ebx_bit_CLFLUSHOPT))		printf("CLFLUSHOPT ");
	if (CHECK(ebx, ef_ebx_bit_CLWB))			printf("CLWB ");
	if (CHECK(ebx, ef_ebx_bit_INTEL_PT))		printf("INTEL_PT ");
	if (CHECK(ebx, ef_ebx_bit_AVX512PF))		printf("AVX512PF ");
	if (CHECK(ebx, ef_ebx_bit_AVX512ER))		printf("AVX512ER ");
	if (CHECK(ebx, ef_ebx_bit_AVX512CD))		printf("AVX512CD ");
	if (CHECK(ebx, ef_ebx_bit_SHA))				printf("SHA ");
	if (CHECK(ebx, ef_ebx_bit_AVX512BW))		printf("AVX512BW ");
	if (CHECK(ebx, ef_ebx_bit_AVX512VL))		printf("AVX512VL ");
	if (CHECK(ecx, ef_ecx_bit_PREFETCHWT1))		printf("PREFETCHWT1 ");
	if (CHECK(ecx, ef_ecx_bit_AVX512VBMI))		printf("AVX512VBMI ");
	if (CHECK(ecx, ef_ecx_bit_UMIP))			printf("UMIP ");
	if (CHECK(ecx, ef_ecx_bit_PKU))				printf("PKU ");
	if (CHECK(ecx, ef_ecx_bit_OSPKE))			printf("OSPKE ");
	if (CHECK(ecx, ef_ecx_bit_AVX512VPOPCNTFQ))	printf("AVX512VPOPCNTFQ ");
	if (CHECK(ecx, ef_ecx_bit_RDPID))			printf("RDPID ");
	if (CHECK(ecx, ef_ecx_bit_SGX_LC))			printf("SGX_LC ");
	if (CHECK(edx, ef_edx_bit_AVX512_4VNIW))	printf("AVX512_4VNIW ");
	if (CHECK(edx, ef_edx_bit_AVX512_4FMAPS))	printf("AVX512_4FMAPS ");
	printf("\n\r");
}

static void print_cpu_vendor(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	volatile uint8_t vendor[16] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	vendor[0] = (ebx >> 0)  & 0xFF;
	vendor[1] = (ebx >> 8)  & 0xFF;
	vendor[2] = (ebx >> 16) & 0xFF;
	vendor[3] = (ebx >> 24) & 0xFF;
	vendor[4] = (edx >> 0)  & 0xFF;
	vendor[5] = (edx >> 8)  & 0xFF;
	vendor[6] = (edx >> 16) & 0xFF;
	vendor[7] = (edx >> 24) & 0xFF;
	vendor[8] =  (ecx >> 0)  & 0xFF;
	vendor[9] =  (ecx >> 8)  & 0xFF;
	vendor[10] = (ecx >> 16) & 0xFF;
	vendor[11] = (ecx >> 24) & 0xFF;

	printf(STR_PREFIX" vendor:  %s\n\r", vendor);

}

static void print_cpu_brand(void){
	uint32_t eax = 0x80000002;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint8_t pname[48] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	pname[0] =   (eax >>  0) & 0xFF;
	pname[1] =   (eax >>  8) & 0xFF;
	pname[2] =   (eax >> 16) & 0xFF;
	pname[3] =   (eax >> 24) & 0xFF;
	pname[4] =   (ebx >>  0) & 0xFF;
	pname[5] =   (ebx >>  8) & 0xFF;
	pname[6] =   (ebx >> 16) & 0xFF;
	pname[7] =   (ebx >> 24) & 0xFF;
	pname[8] =   (ecx >>  0) & 0xFF;
	pname[9] =   (ecx >>  8) & 0xFF;
	pname[10] =  (ecx >> 16) & 0xFF;
	pname[11] =  (ecx >> 24) & 0xFF;
	pname[12] =  (edx >>  0) & 0xFF;
	pname[13] =  (edx >>  8) & 0xFF;
	pname[14] =  (edx >> 16) & 0xFF;
	pname[15] =  (edx >> 24) & 0xFF;

	eax = 0x80000003;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[16] =  (eax >>  0) & 0xFF;
	pname[17] =  (eax >>  8) & 0xFF;
	pname[18] =  (eax >> 16) & 0xFF;
	pname[19] =  (eax >> 24) & 0xFF;
	pname[20] =  (ebx >>  0) & 0xFF;
	pname[21] =  (ebx >>  8) & 0xFF;
	pname[22] =  (ebx >> 16) & 0xFF;
	pname[23] =  (ebx >> 24) & 0xFF;
	pname[24] =  (ecx >>  0) & 0xFF;
	pname[25] =  (ecx >>  8) & 0xFF;
	pname[26] =  (ecx >> 16) & 0xFF;
	pname[27] =  (ecx >> 24) & 0xFF;
	pname[28] =  (edx >>  0) & 0xFF;
	pname[29] =  (edx >>  8) & 0xFF;
	pname[30] =  (edx >> 16) & 0xFF;
	pname[31] =  (edx >> 24) & 0xFF;

	eax = 0x80000004;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[32] =  (eax >>  0) & 0xFF;
	pname[33] =  (eax >>  8) & 0xFF;
	pname[34] =  (eax >> 16) & 0xFF;
	pname[35] =  (eax >> 24) & 0xFF;
	pname[36] =  (ebx >>  0) & 0xFF;
	pname[37] =  (ebx >>  8) & 0xFF;
	pname[38] =  (ebx >> 16) & 0xFF;
	pname[39] =  (ebx >> 24) & 0xFF;
	pname[40] =  (ecx >>  0) & 0xFF;
	pname[41] =  (ecx >>  8) & 0xFF;
	pname[42] =  (ecx >> 16) & 0xFF;
	pname[43] =  (ecx >> 24) & 0xFF;
	pname[44] =  (edx >>  0) & 0xFF;
	pname[45] =  (edx >>  8) & 0xFF;
	pname[46] =  (edx >> 16) & 0xFF;
	pname[47] =  (edx >> 24) & 0xFF;
	
	printf(STR_PREFIX" product: %s\n\r", pname);
}

static void print_cpu_info(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint32_t highest_function, cpuid_value, stepping, model, family, type, extmodel, extfamily;

	cpuid(&eax, &ebx, &ecx, &edx);
	highest_function = eax;

	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	cpuid_value = 	eax;
	stepping = 		eax & 0xF;
	model = 		(eax >> 4) & 0xF;
	family = 		(eax >> 8) & 0xF;
	type = 			(eax >> 12) & 0x3;
	extmodel = 		(eax >> 16) & 0xF;
	extfamily = 	(eax >> 20) & 0xFF;

	printf(STR_PREFIX" hfunc: %d, type=%d family=%d model=%d stepping=%d extfam=%d extmodel=%d\n\r",
					highest_function, cpuid_value,
					type, family, model, stepping, extfamily, extmodel);
}

static void print_cpu_vendor_pt(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	volatile uint8_t vendor[16] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	vendor[0] = (ebx >> 0)  & 0xFF;
	vendor[1] = (ebx >> 8)  & 0xFF;
	vendor[2] = (ebx >> 16) & 0xFF;
	vendor[3] = (ebx >> 24) & 0xFF;
	vendor[4] = (edx >> 0)  & 0xFF;
	vendor[5] = (edx >> 8)  & 0xFF;
	vendor[6] = (edx >> 16) & 0xFF;
	vendor[7] = (edx >> 24) & 0xFF;
	vendor[8] =  (ecx >> 0)  & 0xFF;
	vendor[9] =  (ecx >> 8)  & 0xFF;
	vendor[10] = (ecx >> 16) & 0xFF;
	vendor[11] = (ecx >> 24) & 0xFF;

	printf("vendor:\n\r-> %s\n\r", vendor);

}

static void print_cpu_brand_pt(void){
	uint32_t eax = 0x80000002;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint8_t pname[48] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	pname[0] =   (eax >>  0) & 0xFF;
	pname[1] =   (eax >>  8) & 0xFF;
	pname[2] =   (eax >> 16) & 0xFF;
	pname[3] =   (eax >> 24) & 0xFF;
	pname[4] =   (ebx >>  0) & 0xFF;
	pname[5] =   (ebx >>  8) & 0xFF;
	pname[6] =   (ebx >> 16) & 0xFF;
	pname[7] =   (ebx >> 24) & 0xFF;
	pname[8] =   (ecx >>  0) & 0xFF;
	pname[9] =   (ecx >>  8) & 0xFF;
	pname[10] =  (ecx >> 16) & 0xFF;
	pname[11] =  (ecx >> 24) & 0xFF;
	pname[12] =  (edx >>  0) & 0xFF;
	pname[13] =  (edx >>  8) & 0xFF;
	pname[14] =  (edx >> 16) & 0xFF;
	pname[15] =  (edx >> 24) & 0xFF;

	eax = 0x80000003;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[16] =  (eax >>  0) & 0xFF;
	pname[17] =  (eax >>  8) & 0xFF;
	pname[18] =  (eax >> 16) & 0xFF;
	pname[19] =  (eax >> 24) & 0xFF;
	pname[20] =  (ebx >>  0) & 0xFF;
	pname[21] =  (ebx >>  8) & 0xFF;
	pname[22] =  (ebx >> 16) & 0xFF;
	pname[23] =  (ebx >> 24) & 0xFF;
	pname[24] =  (ecx >>  0) & 0xFF;
	pname[25] =  (ecx >>  8) & 0xFF;
	pname[26] =  (ecx >> 16) & 0xFF;
	pname[27] =  (ecx >> 24) & 0xFF;
	pname[28] =  (edx >>  0) & 0xFF;
	pname[29] =  (edx >>  8) & 0xFF;
	pname[30] =  (edx >> 16) & 0xFF;
	pname[31] =  (edx >> 24) & 0xFF;

	eax = 0x80000004;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[32] =  (eax >>  0) & 0xFF;
	pname[33] =  (eax >>  8) & 0xFF;
	pname[34] =  (eax >> 16) & 0xFF;
	pname[35] =  (eax >> 24) & 0xFF;
	pname[36] =  (ebx >>  0) & 0xFF;
	pname[37] =  (ebx >>  8) & 0xFF;
	pname[38] =  (ebx >> 16) & 0xFF;
	pname[39] =  (ebx >> 24) & 0xFF;
	pname[40] =  (ecx >>  0) & 0xFF;
	pname[41] =  (ecx >>  8) & 0xFF;
	pname[42] =  (ecx >> 16) & 0xFF;
	pname[43] =  (ecx >> 24) & 0xFF;
	pname[44] =  (edx >>  0) & 0xFF;
	pname[45] =  (edx >>  8) & 0xFF;
	pname[46] =  (edx >> 16) & 0xFF;
	pname[47] =  (edx >> 24) & 0xFF;
	
	printf("product:\n\r-> %s\n\r", pname);
}

bool check_support_intel_pt(void){
	tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREEN));

	printf("Checking Intel PT support...\n\r");

	print_cpu_vendor_pt();
	print_cpu_brand_pt();
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(eax < 0x14){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: Not enough CPUID support for Intel PT!\n\r");
		return false;
	}
	printf("[X] CPUID > 0x14 (0x%x)\n\r", eax);

	eax = 0x7;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(!(ebx & BIT(25))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: No Intel PT support!\n\r");
		return false;
	}
	printf("[X] Basic Intel PT\n\r");

	eax = 0x14;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(!(ecx & BIT(0))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: No ToPA support!\n\r");
		return false;
	}
	printf("[X] ToPA Support\n\r");

	if(!(ebx & BIT(0))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: CR3 filtering unsupported!\n\r");
		return false;
	}
	printf("[X] CR3 filtering\n\r");

	if(ecx & BIT(31)){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: IP Payloads are LIP!\n\r");
		return false;
	}
	printf("[X] IP != LIP\n\r");


	if(!(ecx & BIT(1))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: Only one ToPA block supported!\n\r");
		return false;
	}
	printf("[X] Multi ToPA\n\r");


	if(!(ebx & BIT(2))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: No IP-Filtering support!\n\r");
		return false;
	}
	printf("[X] IP-Filtering\n\r");


	uint64_t msr_value = rdmsr64(0x00000485); // MSR_IA32_VMX_MISC  

	if (!(msr_value & BIT(14))){
		tty_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		printf("ERROR: VMX operations are not supported in Intel PT tracing mode!");
		return false; 
	}
	printf("[X] Intel PT + VMX\n\r");


	eax = 0x14;
	ebx = 0;
	ecx = 1;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);

	printf("[X] Trace Regions: %d!\n\r", eax&0x7);


	eax = 0x14;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);

	if(!(ebx & BIT(4))){
		printf("[ ] PTWRITE\n\r");
	}
	else{
		printf("[X] PTWRITE\n\r");
	}

	printf("--> Intel PT is supported!\n\r");
	return true;
}

void print_cpuid(void){
	print_cpu_info();
	print_cpu_vendor();
	print_cpu_brand();
	print_cpu_features();
	print_extended_cpu_features();
}