
 /* credits: wiki.osdev.org, syssec.rub.de (angry_os) */

.section .text
.align 4


/* GDT */

.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    /* Load GDT */
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %ss

    ljmp $0x08, $.flush
.flush:
    ret


/* IDT */

.global idt_load
.type idt_load, @function

idt_load:
    mov 4(%esp), %eax
    lidt (%eax)
    ret


/* ISR */

.macro ISR_NOERR index
    .global _isr\index
    _isr\index:
        /*cli*/
        push $0
        push $\index
        jmp isr_common
.endm

.macro ISR_ERR index
    .global _isr\index
    _isr\index:
        /*cli*/
        push $\index
        jmp isr_common
.endm

.macro IRQ ident offset
    .global _irq\ident
    _irq\ident:
        /*cli*/
        push $0x00
        push $\offset
        jmp irq_common
.endm

.macro MSI_IRQ ident offset
    .global _irq\ident
    _irq\ident:
        /*cli*/
        push $0x00
        push $\offset
        jmp msi_irq_common
.endm

/* Standard X86 interrupt service routines */
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
IRQ 16, 48
IRQ 17, 49
IRQ 18, 50
IRQ 19, 51
IRQ 20, 52
IRQ 21, 53
IRQ 22, 54
IRQ 23, 55
MSI_IRQ 24, 56
MSI_IRQ 25, 57
MSI_IRQ 26, 58
MSI_IRQ 27, 59
MSI_IRQ 28, 60
MSI_IRQ 29, 61
MSI_IRQ 30, 62
MSI_IRQ 31, 63
MSI_IRQ 32, 64
MSI_IRQ 33, 65
MSI_IRQ 34, 66
MSI_IRQ 35, 67
MSI_IRQ 36, 68
MSI_IRQ 37, 69
MSI_IRQ 38, 70
MSI_IRQ 39, 71
MSI_IRQ 40, 72
MSI_IRQ 41, 73
MSI_IRQ 42, 74
MSI_IRQ 43, 75
MSI_IRQ 44, 76
MSI_IRQ 45, 77
MSI_IRQ 46, 78
MSI_IRQ 47, 79
MSI_IRQ 48, 80
MSI_IRQ 49, 81
MSI_IRQ 50, 82
MSI_IRQ 51, 83
MSI_IRQ 52, 84
MSI_IRQ 53, 85
MSI_IRQ 54, 86
MSI_IRQ 55, 87
MSI_IRQ 56, 88
MSI_IRQ 57, 89
MSI_IRQ 58, 90
MSI_IRQ 59, 91
MSI_IRQ 60, 92
MSI_IRQ 61, 93
MSI_IRQ 62, 94
MSI_IRQ 63, 95
MSI_IRQ 64, 96
MSI_IRQ 65, 97
MSI_IRQ 66, 98
MSI_IRQ 67, 99
MSI_IRQ 68, 100
MSI_IRQ 69, 101
MSI_IRQ 70, 102
MSI_IRQ 71, 103
MSI_IRQ 72, 104
MSI_IRQ 73, 105
MSI_IRQ 74, 106
MSI_IRQ 75, 107
MSI_IRQ 76, 108
MSI_IRQ 77, 109
MSI_IRQ 78, 110
MSI_IRQ 79, 111
MSI_IRQ 80, 112
MSI_IRQ 81, 113
MSI_IRQ 82, 114
MSI_IRQ 83, 115
MSI_IRQ 84, 116
MSI_IRQ 85, 117
MSI_IRQ 86, 118
MSI_IRQ 87, 119
MSI_IRQ 88, 120
MSI_IRQ 89, 121
MSI_IRQ 90, 122
MSI_IRQ 91, 123
MSI_IRQ 92, 124
MSI_IRQ 93, 125
MSI_IRQ 94, 126
MSI_IRQ 95, 127
MSI_IRQ 96, 128
MSI_IRQ 97, 129
MSI_IRQ 98, 130
MSI_IRQ 99, 131
MSI_IRQ 100, 132
MSI_IRQ 101, 133
MSI_IRQ 102, 134
MSI_IRQ 103, 135
MSI_IRQ 104, 136
MSI_IRQ 105, 137
MSI_IRQ 106, 138
MSI_IRQ 107, 139
MSI_IRQ 108, 140
MSI_IRQ 109, 141
MSI_IRQ 110, 142
MSI_IRQ 111, 143
MSI_IRQ 112, 144
MSI_IRQ 113, 145
MSI_IRQ 114, 146
MSI_IRQ 115, 147
MSI_IRQ 116, 148
MSI_IRQ 117, 149
MSI_IRQ 118, 150
MSI_IRQ 119, 151
MSI_IRQ 120, 152
MSI_IRQ 121, 153
MSI_IRQ 122, 154
MSI_IRQ 123, 155
MSI_IRQ 124, 156
MSI_IRQ 125, 157
MSI_IRQ 126, 158
MSI_IRQ 127, 159
MSI_IRQ 128, 160
MSI_IRQ 129, 161
MSI_IRQ 130, 162
MSI_IRQ 131, 163
MSI_IRQ 132, 164
MSI_IRQ 133, 165
MSI_IRQ 134, 166
MSI_IRQ 135, 167
MSI_IRQ 136, 168
MSI_IRQ 137, 169
MSI_IRQ 138, 170
MSI_IRQ 139, 171
MSI_IRQ 140, 172
MSI_IRQ 141, 173
MSI_IRQ 142, 174
MSI_IRQ 143, 175
MSI_IRQ 144, 176
MSI_IRQ 145, 177
MSI_IRQ 146, 178
MSI_IRQ 147, 179
MSI_IRQ 148, 180
MSI_IRQ 149, 181
MSI_IRQ 150, 182
MSI_IRQ 151, 183
MSI_IRQ 152, 184
MSI_IRQ 153, 185
MSI_IRQ 154, 186
MSI_IRQ 155, 187
MSI_IRQ 156, 188
MSI_IRQ 157, 189
MSI_IRQ 158, 190
MSI_IRQ 159, 191
MSI_IRQ 160, 192
MSI_IRQ 161, 193
MSI_IRQ 162, 194
MSI_IRQ 163, 195
MSI_IRQ 164, 196
MSI_IRQ 165, 197
MSI_IRQ 166, 198
MSI_IRQ 167, 199
MSI_IRQ 168, 200
MSI_IRQ 169, 201
MSI_IRQ 170, 202
MSI_IRQ 171, 203
MSI_IRQ 172, 204
MSI_IRQ 173, 205
MSI_IRQ 174, 206
MSI_IRQ 175, 207
MSI_IRQ 176, 208
MSI_IRQ 177, 209
MSI_IRQ 178, 210
MSI_IRQ 179, 211
MSI_IRQ 180, 212
MSI_IRQ 181, 213
MSI_IRQ 182, 214
MSI_IRQ 183, 215
MSI_IRQ 184, 216
MSI_IRQ 185, 217
MSI_IRQ 186, 218
MSI_IRQ 187, 219
MSI_IRQ 188, 220
MSI_IRQ 189, 221
MSI_IRQ 190, 222
MSI_IRQ 191, 223
MSI_IRQ 192, 224
MSI_IRQ 193, 225
MSI_IRQ 194, 226
MSI_IRQ 195, 227
MSI_IRQ 196, 228
MSI_IRQ 197, 229
MSI_IRQ 198, 230
MSI_IRQ 199, 231
MSI_IRQ 200, 232
MSI_IRQ 201, 233
MSI_IRQ 202, 234
MSI_IRQ 203, 235
MSI_IRQ 204, 236
MSI_IRQ 205, 237
MSI_IRQ 206, 238
MSI_IRQ 207, 239
MSI_IRQ 208, 240
MSI_IRQ 209, 241
MSI_IRQ 210, 242
MSI_IRQ 211, 243
MSI_IRQ 212, 244
MSI_IRQ 213, 245
MSI_IRQ 214, 246
MSI_IRQ 215, 247
MSI_IRQ 216, 248
MSI_IRQ 217, 249
MSI_IRQ 218, 250
MSI_IRQ 219, 251
MSI_IRQ 220, 252
MSI_IRQ 221, 253
MSI_IRQ 222, 254


.global _irq_spurious
_irq_spurious:
    iret

.extern fault_handler
.type fault_handler, @function

isr_common:
    /* Push all registers */
    pusha

    /* Save segment registers */
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Call fault handler */
    push %esp
    call fault_handler
    add $4, %esp

    /* Restore segment registers */
    pop %gs
    pop %fs
    pop %es
    pop %ds

    /* Restore registers */
    popa
    /* Cleanup error code and ISR # */
    add $8, %esp
    /* pop CS, EIP, EFLAGS, SS and ESP */
    iret

.extern irq_handler
.type irq_handler, @function

irq_common:

    pusha

    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler
    add $4, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds

    popa
    add $8, %esp
    iret

.extern msi_irq_handler
.type msi_irq_handler, @function

msi_irq_common:

    pusha

    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call msi_irq_handler
    add $4, %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds

    popa
    add $8, %esp
    iret


