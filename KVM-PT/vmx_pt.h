#ifndef __VMX_PT_H__
#define __VMX_PT_H__

#include "vmx.h"

struct vcpu_vmx_pt;


void vmx_pt_toggle_entry(struct vcpu_vmx_pt *vmx_pt_config);
void vmx_pt_toggle_exit(struct vcpu_vmx_pt *vmx_pt_config);

bool vmx_pt_multi_cr3_enabled(struct vcpu_vmx_pt *vmx_pt_config);

int vmx_pt_create_fd(struct vcpu_vmx_pt *vmx_pt_config);

bool vmx_pt_vmentry(struct vcpu_vmx_pt *vmx_pt);
void vmx_pt_vmexit(struct vcpu_vmx_pt *vmx_pt);

bool topa_full(struct vcpu_vmx_pt *vmx_pt);

int vmx_pt_setup(struct vcpu_vmx *vmx, struct vcpu_vmx_pt **vmx_pt_config);
void vmx_pt_destroy(struct vcpu_vmx *vmx, struct vcpu_vmx_pt **vmx_pt_config);

void vmx_pt_init(void);
void vmx_pt_exit(void);

int vmx_pt_enabled(void);
int vmx_pt_get_addrn_value(void);

#endif

