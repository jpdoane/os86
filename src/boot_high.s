// Multiboot example from https://wiki.osdev.org/Bare_Bones
// JPD: add paging setup with kernel at 0xc0000000

.intel_syntax noprefix

#include "memmap.h"

/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
 
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot.data
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
 
# Preallocate pages used for paging
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
passthrough_page_table:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.


# The kernel entry point, prior to paging
.section .multiboot.text
.global _start
.type _start, @function
_start:

    #set up boot page directory
	mov edi, boot_page_directory
	mov ecx, 1024

    # set passthrough page table for first 1MB
	mov esi, passthough_page_table      # page table address (passthrough section, phys=virt)
    or  esi, PDE_PRESENT                # set pde enable bit
    mov [edi], esi                      # copy pde to pd

    #initialize passthrough table for memory up to 1MB
	mov edi, passthough_page_table      # page table address (passthrough section, phys=virt)
	mov esi, 0                          # start at 0x0 address
    or  esi, 1                          # set pte enable bit
set_pt1:
    mov [edi], esi                      # copy pte to pt
    inc edi                             # next pte
    add esi, 0x1000                     # increment page address (emable bit still set)
    cmp esi, 0x100000                   # stop after 1MB
    jle set_pt1
    # remaining ptes should be disabled automatically via bss 0-initialization



    # setup second page table for kernel space
    # label is linked to virtual address, but we are not yet paging, so translate to physical address
	mov esi, kernel_page_table-KERNEL_BASE+KERNEL_BASE_PHYS
    or  esi, 1                          # set pde enable bit
    inc edi                             # second pde
    mov [edi], esi                      # copy pde to pd

    # remaining pdes should be disabled automatically via bss 0-initialization


    #initialize passthrough table for kernel mem
	mov edi, kernel_page_table-KERNEL_BASE+KERNEL_BASE_PHYS # page table address, translate to phys addr.
	mov esi, kernel_start-KERNEL_BASE+KERNEL_BASE_PHYS      # start of kernel space, translate to phys addr.
    or  esi, 1                          # set pte enable bit
set_pt2:
    mov [edi], esi                      # copy pte to pt
    inc edi                             # next pte
    add esi, 0x1000                     # increment page address (emable bit still set)
    cmp esi, kernel_stop-KERNEL_BASE+KERNEL_BASE_PHYS # end of kernel space
    jle set_pt2
    # remaining ptes should be disabled automatically via bss 0-initialization


    #enable paging
    mov ecx, kernel_page_directory      #set page dir address
    mov cr3, ecx
    mov ecx, cr0
    or  ecx, 0x80010000                 #set paging ctrl bits
    mov cr0, ecx

    #set stack pointer (we are now using virtual addresses)
	mov esp, stack_top
  
	# pass memory map arguments from bios (stored in eax, ebx) to kernel
    # need to maintain 16-byte alignment so pad to 16 bytes
	push eax	#extra word to maintain 16byte alignement
	push eax	#extra word to maintain 16byte alignement
	push eax	#magic number
	push ebx	#address of memory map table
	call kernel_main

    # loop forever
	cli
1:	hlt
	jmp 1b
 
/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start

// simple kernel stack, 16 byte aligned
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:
