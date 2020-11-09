#include "terminal.h"
#include "kprintf.h"
#include "multiboot.h"

 /* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)  || !defined(__i386__)
#error "This kernel requires ix86-elf cross compiler"
#endif


void kernel_main(multiboot_info_t* mbd, unsigned int magic)
{
	terminal_initialize(&stdout);
    // terminal_setcolor(&stdout, VGA_COLOR_WHITE);
	kprintf("Let's learn about Operating Systems!\n");
	kprintf("Jon Doane, 2020\n");

	if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		kprintf("Error: Invalid memory table!\n");
		return;
	}

	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;
	kprintf("\nAvailable Memory:\n");
	for(unsigned int nn=0; nn<mbd->mmap_length; nn++)
	{
		if(mmap_table[nn].len>0 && mmap_table[nn].size>0)
			kprintf("%.8llp - %.8llp (%8llu kiB) type: %u\n", mmap_table[nn].addr, mmap_table[nn].addr+mmap_table[nn].len - 1, mmap_table[nn].len/1024, mmap_table[nn].type);
		
	}

	//kprintf_test();
}
