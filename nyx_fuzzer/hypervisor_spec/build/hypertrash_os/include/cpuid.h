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

#include "system.h"

#define  CHECK(val, bit) !!(val & bit)
#define  BIT(x) (1 << x)

/*  Feature Bits EAX=1 */
#define f_edx_bit_FPU				BIT(0)
#define f_edx_bit_VME				BIT(1)
#define f_edx_bit_DE				BIT(2)
#define f_edx_bit_PSE				BIT(3)
#define f_edx_bit_TSC				BIT(4)
#define f_edx_bit_MSR				BIT(5)
#define f_edx_bit_PAE				BIT(6)
#define f_edx_bit_MCE				BIT(7)
#define f_edx_bit_CX8				BIT(8)
#define f_edx_bit_APIC				BIT(9)
#define f_edx_bit_SEP				BIT(11)
#define f_edx_bit_MTRR				BIT(12)
#define f_edx_bit_PGE				BIT(13)
#define f_edx_bit_MCA				BIT(14)
#define f_edx_bit_CMOV				BIT(15)
#define f_edx_bit_PAT				BIT(16)
#define f_edx_bit_PSE36				BIT(17)
#define f_edx_bit_PSN				BIT(18)
#define f_edx_bit_CLFSH				BIT(19)
#define f_edx_bit_DS				BIT(21)
#define f_edx_bit_ACPI				BIT(22)
#define f_edx_bit_MMX				BIT(23)
#define f_edx_bit_FXSR				BIT(24)
#define f_edx_bit_SSE				BIT(25)
#define f_edx_bit_SSE2				BIT(26)
#define f_edx_bit_SS				BIT(27)
#define f_edx_bit_HTT				BIT(28)
#define f_edx_bit_TM				BIT(29)
#define f_edx_bit_IA64				BIT(30)
#define f_edx_bit_PBE				BIT(31)

#define f_ecx_bit_SSE3				BIT(0)
#define f_ecx_bit_PCLMULQDQ 		BIT(1)
#define f_ecx_bit_DTES64			BIT(2)
#define f_ecx_bit_MONITOR			BIT(3)
#define f_ecx_bit_DS_CPL			BIT(4)
#define f_ecx_bit_VMX				BIT(5)
#define f_ecx_bit_SMX				BIT(6)
#define f_ecx_bit_EST				BIT(7)
#define f_ecx_bit_TM2				BIT(8)
#define f_ecx_bit_SSSE3				BIT(9)
#define f_ecx_bit_CNXT_ID			BIT(10)
#define f_ecx_bit_SDBG				BIT(11)
#define f_ecx_bit_FMA				BIT(12)
#define f_ecx_bit_CX16				BIT(13)
#define f_ecx_bit_XTPR				BIT(14)
#define f_ecx_bit_PDCM				BIT(15)
#define f_ecx_bit_PCID				BIT(17)
#define f_ecx_bit_DCA				BIT(18)
#define f_ecx_bit_SSE41				BIT(19)
#define f_ecx_bit_SSE42				BIT(20)
#define f_ecx_bit_X2APIC			BIT(21)
#define f_ecx_bit_MOVBE				BIT(22)
#define f_ecx_bit_POPCNT			BIT(23)
#define f_ecx_bit_TSC_DL			BIT(24)
#define f_ecx_bit_AES				BIT(25)
#define f_ecx_bit_XSAVE				BIT(26)
#define f_ecx_bit_OXSAVE			BIT(27)
#define f_ecx_bit_AVX				BIT(28)
#define f_ecx_bit_F16C				BIT(29)
#define f_ecx_bit_RDRND				BIT(30)
#define f_ecx_bit_HYPERVISOR 		BIT(31)

 /* Intel Extended Feature Bits  EAX=7 ECX=0 */
#define ef_ebx_bit_FSGSBASE			BIT(0)
#define ef_ebx_bit_TSC_ADJUST		BIT(1)
#define ef_ebx_bit_SGX				BIT(2)
#define ef_ebx_bit_BM1				BIT(3)
#define ef_ebx_bit_HLE				BIT(4)
#define ef_ebx_bit_AVX				BIT(5)
#define ef_ebx_bit_SMEP				BIT(7)
#define ef_ebx_bit_BMI2				BIT(8)
#define ef_ebx_bit_ERMS				BIT(9)
#define ef_ebx_bit_INVPCID			BIT(10)
#define ef_ebx_bit_RTM				BIT(11)
#define ef_ebx_bit_PQM				BIT(12)
#define ef_ebx_bit_MPX				BIT(14)
#define ef_ebx_bit_PQE				BIT(15)
#define ef_ebx_bit_AVX512F			BIT(16)
#define ef_ebx_bit_AVX512DQ			BIT(17)
#define ef_ebx_bit_RDSEED			BIT(18)
#define ef_ebx_bit_ADX				BIT(19)
#define ef_ebx_bit_SMAP				BIT(20)
#define ef_ebx_bit_AVX512IFMA		BIT(21)
#define ef_ebx_bit_PCOMMIT			BIT(22)
#define ef_ebx_bit_CLFLUSHOPT		BIT(23)
#define ef_ebx_bit_CLWB				BIT(24)
#define ef_ebx_bit_INTEL_PT			BIT(25)
#define ef_ebx_bit_AVX512PF			BIT(26)
#define ef_ebx_bit_AVX512ER			BIT(27)
#define ef_ebx_bit_AVX512CD			BIT(28)
#define ef_ebx_bit_SHA				BIT(29)
#define ef_ebx_bit_AVX512BW			BIT(30)
#define ef_ebx_bit_AVX512VL			BIT(31)

#define ef_ecx_bit_PREFETCHWT1		BIT(0)
#define ef_ecx_bit_AVX512VBMI		BIT(1)
#define ef_ecx_bit_UMIP				BIT(2)
#define ef_ecx_bit_PKU				BIT(3)
#define ef_ecx_bit_OSPKE			BIT(4)
#define ef_ecx_bit_AVX512VPOPCNTFQ	BIT(14)
#define ef_ecx_bit_RDPID			BIT(22)
#define ef_ecx_bit_SGX_LC			BIT(30)

#define ef_edx_bit_AVX512_4VNIW		BIT(2)
#define ef_edx_bit_AVX512_4FMAPS	BIT(3)

 /* AMD Extended Feature Bits  EAX=80000001h */
/*  Feature Bits EAX=1 */
#define aef_edx_bit_FPU				BIT(0)
#define aef_edx_bit_VME				BIT(1)
#define aef_edx_bit_DE				BIT(2)
#define aef_edx_bit_PSE				BIT(3)
#define aef_edx_bit_TSC				BIT(4)
#define aef_edx_bit_MSR				BIT(5)
#define aef_edx_bit_PAE				BIT(6)
#define aef_edx_bit_MCE				BIT(7)
#define aef_edx_bit_CX8				BIT(8)
#define aef_edx_bit_APIC			BIT(9)
#define aef_edx_bit_SYSCALL			BIT(11)
#define aef_edx_bit_MTRR			BIT(12)
#define aef_edx_bit_PGE				BIT(13)
#define aef_edx_bit_MCA				BIT(14)
#define aef_edx_bit_CMOV			BIT(15)
#define aef_edx_bit_PAT				BIT(16)
#define aef_edx_bit_PSE36			BIT(17)
#define aef_edx_bit_MP				BIT(19)
#define aef_edx_bit_NX				BIT(20)
#define aef_edx_bit_MMXEXT			BIT(22)
#define aef_edx_bit_MMX				BIT(23)
#define aef_edx_bit_FXSR			BIT(24)
#define aef_edx_bit_FXSR_OPT		BIT(25)
#define aef_edx_bit_PDPE1GB			BIT(26)
#define aef_edx_bit_RDTSCP			BIT(27)
#define aef_edx_bit_LM				BIT(29)
#define aef_edx_bit_3DNOWEXT		BIT(30)
#define aef_edx_bit_3DNOW			BIT(31)
#define aef_ecx_bit_LAHF_LM			BIT(0)
#define aef_ecx_bit_CMP_LEGACY		BIT(1)
#define aef_ecx_bit_SVM				BIT(2)
#define aef_ecx_bit_EXTAPIC			BIT(3)
#define aef_ecx_bit_CR8_LEGACY		BIT(4)
#define aef_ecx_bit_ABM				BIT(5)
#define aef_ecx_bit_SSE4A			BIT(6)
#define aef_ecx_bit_MISALIGNSSE		BIT(7)
#define aef_ecx_bit_3DNOWPREFETCH	BIT(8)
#define aef_ecx_bit_OSVW			BIT(9)
#define aef_ecx_bit_IBS				BIT(10)
#define aef_ecx_bit_XOP				BIT(11)
#define aef_ecx_bit_SKINIT			BIT(12)
#define aef_ecx_bit_WDT				BIT(13)
#define aef_ecx_bit_LWP				BIT(15)
#define aef_ecx_bit_FMA4			BIT(16)
#define aef_ecx_bit_TCE				BIT(17)
#define aef_ecx_bit_NODEID_MSR		BIT(19)
#define aef_ecx_bit_TBM				BIT(21)
#define aef_ecx_bit_TOPEXT			BIT(22)
#define aef_ecx_bit_PERFCTR_CORE	BIT(23)
#define aef_ecx_bit_PERFCTR_NB		BIT(24)
#define aef_ecx_bit_DBX				BIT(26)
#define aef_ecx_bit_PERFSC			BIT(27)
#define aef_ecx_bit_PCX_L2I			BIT(28)

static inline void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx){
	uint32_t ain = *eax;
	uint32_t bin = *ebx;
	uint32_t cin = *ecx;
	uint32_t din = *edx;

	__asm__ ("cpuid\n\t"
	   : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
	   : "0" (ain), "1" (bin), "2" (cin), "3" (din));
}

void print_cpuid(void);
bool supports_vmx(void);
