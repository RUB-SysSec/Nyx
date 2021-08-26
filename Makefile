# SPDX-License-Identifier: GPL-2.0

ccflags-y += -Iarch/x86/kvm -Iinclude/ -DCONFIG_KVM_VMX_PT -DCONFIG_KVM_VMX_FDL -I/home/kafl/release/KVM-PT

CFLAGS_x86.o := -I.
CFLAGS_svm.o := -I.
CFLAGS_vmx.o := -I.

KVM := ./virt/kvm


dell-y			+= $(KVM)/kvm_main.o $(KVM)/coalesced_mmio.o \
				$(KVM)/eventfd.o $(KVM)/irqchip.o $(KVM)/vfio.o
dell-$(CONFIG_KVM_ASYNC_PF)	+= $(KVM)/async_pf.o

dell-y			+= x86.o mmu.o emulate.o i8259.o irq.o lapic.o \
			   i8254.o ioapic.o irq_comm.o cpuid.o pmu.o mtrr.o \
			   hyperv.o page_track.o debugfs.o


#dell-intel-y		+= vmx.o pmu_intel.o
#dell-intel-$(CONFIG_KVM_VMX_PT) += vmx_pt.o
#dell-intel-$(CONFIG_KVM_VMX_PT) += vmx_fdl.o


dell-y            += vmx.o pmu_intel.o
dell-y += vmx_pt.o
dell-y += vmx_fdl.o

#dell-amd-y		+= svm.o pmu_amd.o

obj-$(CONFIG_KVM)	+= dell.o
#obj-$(CONFIG_KVM_INTEL)	+= dell-intel.o
#obj-$(CONFIG_KVM_AMD)	+= dell-amd.o
