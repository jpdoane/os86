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
	terminal_init(&stdout);
    // terminal_setcolor(&stdout, VGA_COLOR_WHITE);
	kprintf("Let's learn about Operating Systems!\n");
	kprintf("Jon Doane, 2020\n\n");
	
    int result = 0;

    result = result || print_testresult(init_gdt(), "Initialize descriptor tables");

    result = result || print_testresult(init_interrupts(), "Initialize interrupts");
    result = result || print_testresult(global_memory_init(mbd), "Initialize memory");
    result = result || print_testresult(initialize_multitasking(), "Initialize multitasking");

    result = result || print_testresult(test_kmalloc(), "kmalloc() unit tests");
    result = result || print_testresult(test_multitasking(), "Multitasking unit tests");

    print_testresult(result, "Startup successful");

	while(1);
}
