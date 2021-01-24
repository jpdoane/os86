#include "terminal.h"
#include "kprintf.h"
#include "paging.h"
#include "multiboot.h"
#include "mman.h"
#include "interrupt.h"
#include "gdt.h"
#include "task.h"
#include "regs.h"
#include "test.h"

 /* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)  || !defined(__i386__)
#error "This kernel requires ix86-elf cross compiler"
#endif

void kernel_main(multiboot_info_t* mbd)
{
	init_gdt();
	init_interrupts();

	terminal_init(&stdout);

    // terminal_setcolor(&stdout, VGA_COLOR_WHITE);
	kprintf("Let's learn about Operating Systems!\n");
	kprintf("Jon Doane, 2020\n\n");

	print_memory_table(mbd);
	memory_init(mbd);	//after this the multiboot structure is unmapped
	initialize_multitasking();

	// // test page fault
	// int* bad_ptr = 0xe0000000;
	// int x = *bad_ptr;

	// print registers
    // reg_t regs;
    // getregs(&regs);
	// printregs(&regs);


	if(test_kmalloc())
		kprintf("Malloc tests FAILED!\n");
	else
		kprintf("Malloc tests PASSED!\n");

	if(test_multitasking())
		kprintf("Multitasking tests FAILED!\n");
	else
		kprintf("Multitasking tests PASSED!\n");

	while(1);
}
