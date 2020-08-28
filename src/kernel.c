#include "terminal.h"
#include "kprintf.h"

 /* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)  || !defined(__i386__)
#error "This kernel requires ix86-elf cross compiler"
#endif

void kernel_main(void) 
{
	terminal_initialize(&stdout);

    terminal_setcolor(&stdout, VGA_COLOR_WHITE);
	kprintf("Let's learn about Operating Systems!\n");
    terminal_setcolor(&stdout, VGA_COLOR_DARK_GREY);
	kprintf("Jon Doane, 2020\n");
    terminal_setcolor(&stdout, VGA_COLOR_WHITE);

	kprintf_test();
}
