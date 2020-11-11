ENTRY(_start)
 
SECTIONS
{
	_kernel_start_phys = 0x00101000;
	_kernel_start = 0xC0000000;
	_kernel_virt_offset = _kernel_start - _kernel_start_phys;

	/* Multiboot lives at 0x00100000 in the physical address space. */
	. = 0x00100000;
	.text.boot ALIGN (4K) :
	{
		KEEP(*(.mbheader))
		*(.text.boot)
	}

	/* Kernel lives at 0xC0000000 in the virtual address space. */
	. = _kernel_start;
	.text ALIGN (4K) : AT (ADDR (.text) - _kernel_virt_offset )
	{
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - _kernel_virt_offset )
	{
		*(.rodata)
	}
	.data ALIGN (4K) : AT (ADDR (.data) - _kernel_virt_offset )
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - _kernel_virt_offset )
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
		*(.page_tables)
	}
	/* Add a symbol that indicates the end address of the kernel. */
	_kernel_end = .;
	_kernel_end_phys = _kernel_end - _kernel_virt_offset;

	.comment : { *(.comment) }    

}
