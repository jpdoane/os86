.intel_syntax noprefix

#include "gdt.h"

.section .text
#void load_gdt(gdt_description_t* gdt_desc);
.globl load_gdt
.align 4
.type load_gdt, @function
load_gdt:
    # move arg 1 (page dir) to cr3
    mov eax, [ebp+8]
    lgdt [eax]

    jmp 0x08:complete_flush
 
complete_flush:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
