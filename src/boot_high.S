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
	.align PAGING_PD_SIZE
boot_page_directory:
	.skip PAGING_PD_SIZE
boot_pt_mb:
	.skip PAGING_PT_SIZE
boot_pt_kernel:
	.skip PAGING_PT_SIZE
# Further page tables may be required if the kernel grows beyond 1 MiB.

# The kernel entry point, prior to paging
.section .text.boot
.global _start
.type _start, @function
_start:
	
	cld 	#loops increment

	# mov [stack_top-4], eax  #save magic number
	# mov [stack_top-8], ebx  #save address of memory map table
	mov [KERNEL_VIRT_TO_PHYS(stack_top)-4], eax  #save magic number
	mov [KERNEL_VIRT_TO_PHYS(stack_top)-8], ebx  #save address of memory map table

    #boot page directory
	lea edi, KERNEL_VIRT_TO_PHYS(boot_page_directory)

    # setup page table for low mem on boot
	# this is important b/c we still need to map the mb addr space until we jump to high
	lea esi, KERNEL_VIRT_TO_PHYS(boot_pt_mb)
    or  esi, PDE_PRESENT | PDE_READWRITE	# set pde enable bit
    mov [edi], esi                      # copy pde
	# #compute page dir offset for boot page
    # lea eax, .text.boot  				# mb address
    # shr eax, 22                         # pd_index = (addr>>22)
    # mov ebx, 4
    # mul ebx                             # pd offset = pd_index*4
    # add edi, eax                        # select pde
    # mov [edi], esi                      # copy pde

    #populate multiboot page
	lea edi, KERNEL_VIRT_TO_PHYS(boot_pt_mb)	#page table addr
	mov eax, PTE_PRESENT | PTE_READWRITE		#start at addr 0, with present bit set
	mov ecx, PAGING_NUM_PTE
    # lea esi, .text.boot 						# mb addr
	# and esi, 0xffff000
    # or  esi, PTE_PRESENT						# set pte enable
    # mov eax, esi
    # shr eax, 12                         		# pt_index = (addr>>12) & 0x3ff
	# and eax, 0x3ff								
    # mov ebx, 4
    # mul ebx                             		# pt offset = pt_index*4
    # add edi, eax                         		#
	# mov [edi], esi	
.nextEntry_low:
    stosd										# [edi++] = eax
    add eax,0x1000								# next page
    loop .nextEntry_low						# loop while (--ecx) != 0



    # setup page table for high mem on boot
	lea esi, KERNEL_VIRT_TO_PHYS(boot_pt_kernel)		#page table addr
    or  esi, PDE_PRESENT | PTE_READWRITE                # pde = addr | enable | write
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
	lea ecx, _kernel_end
	sub ecx, ebx
    shr ecx, 12

    #populate kernel page table
    lea eax, _kernel_start 						# kernel virtual address
	#compute initial page table offset for first page
    shr eax, 12                         		# pt_index = (addr>>12) & 0x3ff
	and eax, 0x3ff								
    mov ebx, 4
    mul ebx                             		# pt offset = pt_index*4
	lea edi, KERNEL_VIRT_TO_PHYS(boot_pt_kernel)	#page table addr
    add edi, eax                         		#

    lea eax, KERNEL_VIRT_TO_PHYS(_kernel_start) # kernel physical address
    or  eax, PTE_PRESENT | PTE_READWRITE		# pte = page phys addr | enable,write bits
.nextEntry_high:
    stosd										# [edi++] = eax
    add eax,0x1000								# next page
    loop .nextEntry_high						# loop while (--ecx) != 0

    # self-map last pde
	lea esi, KERNEL_VIRT_TO_PHYS(boot_page_directory)
	mov edi, esi
	add edi, PAGING_PD_SIZE-4           # last pde
    or  esi, PDE_PRESENT | PTE_READWRITE # set pde enable bit
    mov [edi], esi                      # copy to last pde


_enable_paging:
    #enable paging
    lea eax, KERNEL_VIRT_TO_PHYS(boot_page_directory)        #set page dir address
    mov cr3, eax
    mov eax, cr0
    or  eax, 0x80010000                 #set paging ctrl bits
    mov cr0, eax

	mov eax, [stack_top-4]  #save magic number
	mov ebx, [stack_top-8]  #save address of memory map table

#     #set stack pointer (we are now using virtual addresses)
	mov [stack_top-4], esp 
	lea esp, stack_top


	# #disable mb page
	# lea edi, boot_page_directory
    # lea eax, .text.boot  				# mb address
    # shr eax, 22                         # pd_index = (addr>>22)
    # mov ebx, 4
    # mul ebx                             # pd offset = pd_index*4
    # add edi, eax                        # select pde
	# xor eax, eax
    # mov [edi], eax                      	# clear pde

# 	# pass memory map arguments from bios (stored in eax, ebx) to kernel
#     # need to maintain 16-byte alignment so pad to 16 bytes
	lea edi, boot_page_directory
	push eax	#extra word to maintain 16byte alignement
	push edi	#page directory
	push eax	#magic number
	push ebx	#address of memory map table
	call kernel_main

	cli
1:	hlt
	jmp 1b


/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start
