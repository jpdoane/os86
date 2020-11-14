// Multiboot example from https://wiki.osdev.org/Bare_Bones
// JPD: add paging setup with kernel at 0xc0000000

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

 // simple kernel stack, 16 byte aligned
//.section .bootstrap_stack, "aw", @nobits
.section .bootstrap_stack
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging
.section .page_tables, "aw", @nobits
	.align PAGE_SIZE
boot_page_directory:
	.skip PAGE_SIZE
boot_pt_kernel:
	.skip PAGE_SIZE
# Further page tables may be required if the kernel grows beyond 1 MiB.

# The kernel entry point, prior to paging
.section .text.boot
.global _start
.type _start, @function
_start:
	
	# test for multiboot magic number in eax
	cmp eax, MULTIBOOT_BOOTLOADER_MAGIC
	jne kpanic

    #set stack pointer
	lea esp, KERNEL_VIRT_TO_PHYS(stack_top)
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

    # setup page table for high mem on boot
	lea esi, KERNEL_VIRT_TO_PHYS(boot_pt_kernel)		#page table addr
    or  esi, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE                # pde = addr | enable | write
	# compute offset into directory
    lea eax, _kernel_start           	# kernel virtual address
    shr eax, 22                         # pd_index = (addr>>22)
    mov ebx, 4
    mul ebx                             # pd_offset = pd_index*4
	lea edi, KERNEL_VIRT_TO_PHYS(boot_page_directory)	#page dir addr
    add edi, eax                        # 
    mov [edi], esi                      # pd[pd_index] = *(pd + pd_offset) = pde

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
	lea edi, KERNEL_VIRT_TO_PHYS(boot_pt_kernel)	#page table addr
    add edi, eax                         		#

    lea eax, KERNEL_VIRT_TO_PHYS(_kernel_start) # kernel physical address
    or  eax, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE		# pte = page phys addr | enable,write bits
	cld 										#loop increments edi
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
	lea esp, stack_top

 	# jump to main kernel code
	push eax	#extra push to maintain 16b stack alignment
	push eax	#extra push to maintain 16b stack alignment
	push eax	#extra push to maintain 16b stack alignment
	push ebx	#address of memory map table
	call kernel_main

	cli
kpanic:	hlt
	jmp kpanic


/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start
