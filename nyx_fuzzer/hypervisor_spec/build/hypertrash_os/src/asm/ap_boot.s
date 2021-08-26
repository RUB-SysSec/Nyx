.globl __ap_boot

.align 4096
.code16

.extern ap_main
.type ap_main, @function

__ap_boot:
	cli							/* welcome in real mode */
	
	mov %cs, %ax	
	mov %ax, %ds

	lgdtl  __gdt_Ptr -  __ap_boot + 2

	mov %cr0, %eax
	or $1, %eax
	mov %eax, %cr0

	ljmpl $0x8, $1f				/* switch to protected mode */

.code32

1:
	 mov $0x10, %eax
	 mov %ax, %ds
	 mov %ax, %es
	 mov %ax, %fs
	 mov %ax, %gs
	 mov %ax, %ss

	 mov ap_stack, %esp
	 call ap_main

__gdt_Ptr:
	.long 0xFFFF0000
	.long gdt 					/* reuse the BSP GDT */