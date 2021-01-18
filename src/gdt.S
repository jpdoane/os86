.intel_syntax noprefix

#include "gdt.h"

.section .text
#void load_gdt()
.globl load_gdt
.extern gdtd
.align 4
.type load_gdt, @function
load_gdt:
    lgdt [gdtd]

    jmp KERNEL_CODE_SEGMENT:complete_flush
complete_flush:
    mov ax, KERNEL_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret

#void load_tss();
.globl load_tss
.align 4
.type load_tss, @function
load_tss:
    mov ax, TSS_SEGMENT
    or ax, 0x3 #set 1st 2 bits for user priv level
    ltr ax
    ret