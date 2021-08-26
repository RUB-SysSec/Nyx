 /* credits: wiki.osdev.org, syssec.rub.de (angry_os) */

multiboot_header:
    .long 0xe85250d6                                                     /* magic */
    .long 0                                                              /* ISA: i386 */
    .long   multiboot_header_end - multiboot_header                      /* Header length.  */
    .long   -(0xe85250d6 + (multiboot_header_end - multiboot_header))    /* checksum */
    .short 0
    .short 0
    .long 8
multiboot_header_end:

/* .stack resides in .bss */
.section .stack, "aw", @nobits
stack_bottom:
.skip 16384 /* 16KiB */
stack_top:

.section .text

.global _start
.type _start, @function


.extern kernel_main
.type kernel_main, @function

.extern printf
.type tty_printf, @function

_start:
    /* Setup our stack */
    mov $stack_top, %esp

    /* Make sure our stack is 16-byte aligned */
    and $-16, %esp

    pushl %esp
    pushl %eax /* Multiboot header magic */
    pushl %ebx /* Multiboot header pointer */

    cli
    call kernel_main

    cli
    pushl   $halt_message
    call    tty_printf
hang:
    hlt
    jmp hang

halt_message:
    .asciz  "Halted."
