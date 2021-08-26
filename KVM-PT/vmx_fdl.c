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

#include "vmx_fdl.h"
#include <linux/types.h>
#include <asm/nmi.h>
#include <asm/page.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/anon_inodes.h>
#include <linux/vmalloc.h>


/*===========================================================================* 
 *                              Fast Reload Mechanism                        * 
 *===========================================================================*/ 

#define MAX_DIRTY_BUFFER	0x1000000000	/* 64GB max */

struct vm_vmx_fdl{
	u64 index; 
	void* buf;
	u64 buf_size; 
	u64 alloc_size;
};

static void vmx_fdl_set_addr(struct vm_vmx_fdl* data, u64 gpfn){
	if(data->buf && data->index < data->buf_size){
		((u64*)(data->buf))[(data->index)] = gpfn;
		//printk("Writing %llx [%d]\n", gpfn, data->index);
		data->index += 1;
	}
}

void vmx_fdl_set_addr_kvm(void* data, u64 gpa){
	struct vm_vmx_fdl* fdl_data = ((struct kvm*)data)->arch.fdl_opaque;
	struct kvm_memory_slot* slot = gfn_to_memslot(((struct kvm*)data), gpa>>12);
	if (slot && slot->dirty_bitmap) {
		u64 rel_gfn = (gpa>>12) - slot->base_gfn;
		if(!test_bit_le(rel_gfn, slot->dirty_bitmap)){
			vmx_fdl_set_addr(fdl_data, gpa&0xFFFFFFFFFFFFF000);
		}
	}
}

void vmx_fdl_set_addr_vpcu(void* data, u64 gpa){
	vmx_fdl_set_addr_kvm((void*)(((struct kvm_vcpu*)data)->kvm), gpa);
}

static int vmx_fdl_release(struct inode *inode, struct file *filp)
{
#ifdef DEBUG
	PRINT_INFO("fast_reload_release: file closed ...");
#endif
	return 0;
}

static long vmx_fdl_realloc_memory(struct vm_vmx_fdl* data, u64 size){
	long r = -EINVAL;
	if(size <= MAX_DIRTY_BUFFER){
		if(!data->buf){
			if(size < 8){
				return r;
			}
			data->buf_size = size;
			data->alloc_size = data->buf_size + (PAGE_SIZE-(data->buf_size%PAGE_SIZE));
			data->buf = vmalloc(data->alloc_size);
			if (!data->buf){
				/* allocation failed, dude! */
				return r;
			}
			memset(data->buf, 0x00, data->alloc_size);
			data->index = 0;
			r = 0;
		}
	}
	return r; 
}

static long vmx_fdl_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg){
	long r = -EINVAL;
	struct vm_vmx_fdl *data = filp->private_data;

	if(!data){
		return r;
	}

	switch (ioctl) {
		case KVM_VMX_FDL_SET:
			r = vmx_fdl_realloc_memory(data, arg);
			break;

		case KVM_VMX_FDL_FLUSH:
			/* reconfigure the index to zero */
			data->index = 0;
			r = 0;
			break;

		case KVM_VMX_FDL_GET_INDEX:
			r = data->index;
			data->index = 0;	/* flush */
			break;

		default:
			break;
	}
	return r; 
}

static int vmx_fdl_mmap(struct file *filp, struct vm_area_struct *vma)
{	
	u64 uaddr, vaddr;
	struct vm_vmx_fdl *data = filp->private_data;
	struct page * pageptr;

	if ((vma->vm_end-vma->vm_start) > (data->alloc_size)){
		return -EINVAL;
	}
	vma->vm_flags = (VM_READ | VM_SHARED | VM_DENYWRITE); 
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	uaddr = (u64)vma->vm_start;
	vaddr = (u64)data->buf;
	do {
	    pageptr = vmalloc_to_page((void*)vaddr);
	    vm_insert_page(vma, uaddr, pageptr);
	    vaddr += PAGE_SIZE;
	    uaddr += PAGE_SIZE;
	} while(uaddr < vma->vm_end || vaddr < data->alloc_size);

	return 0;   
}

static struct file_operations vmx_fdl_fops = {
	.release        = vmx_fdl_release,
	.unlocked_ioctl = vmx_fdl_ioctl, 
	.mmap           = vmx_fdl_mmap,
	.llseek         = noop_llseek,
};

int vmx_fdl_create_fd(void* vmx_fdl_opaque){
	if(vmx_fdl_opaque){
		return anon_inode_getfd("vmx-fdl", &vmx_fdl_fops, vmx_fdl_opaque, O_RDWR | O_CLOEXEC); 
	}
	return 0;
}

void vmx_fdl_setup(void** vmx_fdl_opaque){
	if(!(*vmx_fdl_opaque)){
		*vmx_fdl_opaque = (void*)kmalloc(sizeof(struct vm_vmx_fdl), GFP_KERNEL);
		((struct vm_vmx_fdl*)(*vmx_fdl_opaque))->index = 0;
		((struct vm_vmx_fdl*)(*vmx_fdl_opaque))->buf = NULL;
		((struct vm_vmx_fdl*)(*vmx_fdl_opaque))->buf_size = 0;
	}
}

void vmx_fdl_destroy(void* vmx_fdl_opaque){ 
	
	if(vmx_fdl_opaque){
		if(((struct vm_vmx_fdl*)(vmx_fdl_opaque))->buf){
			vfree(((struct vm_vmx_fdl*)(vmx_fdl_opaque))->buf);
		}
		kfree(vmx_fdl_opaque);
	}
}

