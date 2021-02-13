.intel_syntax noprefix

#include "paging.h"

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
.section .mbheader
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss

 // initial tcb
# .align PAGE_SIZE
# .extern _boot_tcb
# .global _boot_tcb
# _boot_tcb:
# .skip PAGE_SIZE
# _boot_page_directory:
# .skip PAGE_SIZE
# _boot_page_directory:
# .skip PAGE_SIZE
# .extern _boot_kstack_base
# .global _boot_kstack_base
# _boot_kstack_base:
# .skip 3 * PAGE_SIZE
# .extern _boot_kstack_top
# .global _boot_kstack_top
# _boot_kstack_top:

# Preallocate pages used for paging
.align PAGE_SIZE
kernel_page_tables:
.skip PAGE_SIZE*255 # space for 255 kernel page tables


.extern boot_task
#define boot_page_directory boot_task+PAGE_SIZE*1
#define boot_kstack_top boot_task+PAGE_SIZE*4

# The kernel entry point, prior to paging
.section .text.boot
.global _start
.type _start, @function
_start:	
    cli  				#disable interrupts

	# test for multiboot magic number in eax
	cmp eax, MULTIBOOT_BOOTLOADER_MAGIC
	jne kpanic

    #set stack pointer
	lea esp, KERNEL_VIRT_TO_PHYS(boot_kstack_top); //top of first kstack

	push ebx	#save address of memory map table

    #boot page directory
	lea edi, KERNEL_VIRT_TO_PHYS(boot_page_directory)

    #identity map the entire first 4MiB with first pde
	#this ensures that the physical kernel addresses are valid after we enable paging but before we've jumped to high mem
	mov eax, PAGE_FLAG_PRESENT | PAGE_FLAG_SIZE | PAGE_FLAG_WRITE				#use large size pde at addr = 0 
	mov [edi], eax
	#enable pse (large pages)
	mov eax, cr4
	or eax, 0x00000010 	
	mov cr4, eax



# Initialize page directory with (empty) kernel page tables
.init_pagedir:
	# kernel memory spans the top 1GB from 0xc0000000 to 0xffffffff.
	# So we need to initialize the last 256 (last quarter) of pdes (page tables)
	# however, the last page table actually maps back to the PD and is used to write to the page tables themselves
	# so there are are only 255 usable kernel page tables

	# edi <- address of first PDE of kernel space
    lea eax, [KERNEL_BASE]				# kernel base virtual address
    shr eax, 22                         # pd_index = (addr>>22)
    mov ebx, 4
    mul ebx                             # pd_offset = pd_index*4
	lea edi, KERNEL_VIRT_TO_PHYS(boot_page_directory)	#page dir addr
    add edi, eax                        # 

	# eax <- initial pde for kernel space
	lea eax, KERNEL_VIRT_TO_PHYS(kernel_page_tables)			#address of first kernel page table
    or  eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE            # pde = addr | enable | write

	# ecx <- number of kernel pdes
    mov ecx, 255
	cld 										#loop increments edi

.nextpde:
    stosd										# [edi++] = eax
    add eax,0x1000								# next pde
    loop .nextpde								# loop while (--ecx) != 0


.init_pagetables:
#intialize the first kernel page table with initial (static) kernel memory
#we are assuming here that the static kernel is <4MB and will fit in single page table
	# compute offset of into directory for kernel page table
    lea eax, _kernel_start           		# kernel base virtual address
    shr eax, 22                         # pd_index = (addr>>22)
    mov ebx, 4
    mul ebx                             # pd_offset = pd_index*4
	lea edi, KERNEL_VIRT_TO_PHYS(boot_page_directory)	#page dir addr
    add edi, eax                        # 

	#compute number of kernel pages, store in ecx
	lea ebx, _kernel_start
    shr ebx, 12
	lea ecx, _kernel_end
	add ecx, PAGE_SIZE-1
    shr ecx, 12
	sub ecx, ebx

    #populate kernel page table
    lea eax, _kernel_start 						# kernel virtual address
	#compute initial page table offset for first page
	and eax, PAGE_PTE_MASK
    shr eax, 12                         		# pt_index = (addr>>12) & 0x3ff
    mov ebx, 4
    mul ebx                             		# pt offset = pt_index*4
	lea edi, KERNEL_VIRT_TO_PHYS(kernel_page_tables)	#page table addr
    add edi, eax                         		#

    lea eax, KERNEL_VIRT_TO_PHYS(_kernel_start) # kernel physical address
    or  eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE		# pte = page phys addr | enable,write bits

.nextpte:
    stosd										# [edi++] = eax
    add eax,0x1000								# next page
    loop .nextpte								# loop while (--ecx) != 0

    # self-map last pde
	lea esi, KERNEL_VIRT_TO_PHYS(boot_page_directory)
	mov edi, esi
	add edi, PAGE_SIZE-4           # last pde
    or  esi, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE # set pde enable bit
    mov [edi], esi                      # copy to last pde



	pop ebx 	#get address of memory map table off the stack
				#do this before we enable paging to start with empty stack to make it easier...

_enable_paging:

    #enable paging
    lea eax, KERNEL_VIRT_TO_PHYS(boot_page_directory)
    mov cr3, eax						#set page dir addr
    mov eax, cr0
    or  eax, 0x80010000                 #set paging ctrl bits
    mov cr0, eax

	# we are now paging, using virtual addresses

    #update stack pointer to virtual addr
	lea esp, boot_kstack_top

 	# jump to main kernel code
	push eax	#extra push to maintain 16B stack alignment
	push eax	#extra push to maintain 16B stack alignment
	push eax	#extra push to maintain 16B stack alignment
	push ebx	#address of memory map table
	call kernel_main

	cli
kpanic:	hlt
	jmp kpanic
