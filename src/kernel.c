#include "terminal.h"
#include "kprintf.h"
#include "multiboot.h"
// #include "memman.h"

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

	size_t mem_size = memory_table(mbd, magic);
	if(mem_size==0)
	{
		kprintf("Error: Invalid memory table!\n");
		return;
	}
	else
	{
		kprintf("Found %u MiB availble free memory\n", mem_size/(1<<20));
		print_memory_table(mbd);
	}
	

	kprintf_test();

	
}
