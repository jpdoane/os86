#include "terminal.h"
#include "kprintf.h"
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

#define RUN_UNIT_TESTS

void kernel_main(multiboot_info_t* mbd)
{
    //this relocates the stack, so this needs to be one of the first calls
    init_kernel_memory(mbd);

	terminal_init(&stdout);
    // terminal_setcolor(&stdout, VGA_COLOR_WHITE);
	kprintf("Let's learn about Operating Systems!\n");
	kprintf("Jon Doane, 2020\n\n");
	
    int result = 0;

    result = result || print_testresult(init_gdt(), "Initialize descriptor tables");
    result = result || print_testresult(init_interrupts(), "Initialize interrupts");
    result = result || print_testresult(initialize_multitasking(), "Initialize multitasking");

    if(result)
        print_testresult(result, "Errors during startup");
    else
        print_testresult(result, "Startup successful");


#ifdef RUN_UNIT_TESTS
    run_unit_tests();
#endif

    
	while(1);
}
