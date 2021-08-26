/*
 * Kernel AFL Fast Dirty Page Logging Driver (KVM Extension)
 * (c) Sergej Schumilo 2017 - sergej@schumilo.de
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef __VMX_FDL_H__
#define __VMX_FDL_H__

#include "header/uapi/linux/kvm.h"
//#include <linux/kvm.h>
#include <linux/types.h>
#include "header/linux/kvm_host.h"
//#include <linux/kvm_host.h>


void vmx_fdl_set_addr_kvm(void* data, u64 gpa);
void vmx_fdl_set_addr_vpcu(void* data, u64 gpa);
void vmx_fdl_setup(void** vmx_fdl_opaque);
int vmx_fdl_create_fd(void* vmx_fdl_opaque);
void vmx_fdl_destroy(void* vmx_fdl_opaque);

#endif

