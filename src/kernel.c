#include "terminal.h"
#include "kprintf.h"
#include "paging.h"
#include "multiboot.h"
#include "mman.h"
#include "interrupt.h"
#include "gdt.h"
#include "task.h"
#include "regs.h"

 /* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)  || !defined(__i386__)
#error "This kernel requires ix86-elf cross compiler"
#endif

int32_t test_multitasking2()
{
	int nn;
	for(nn=0;nn<5;nn++)
	{
		kprintf("task 2, count %d, [%d active tasks]\n", nn, num_tasks());
		yield();
	}
	return nn;
}

void test_multitasking1()
{
	task_control_block_t* task2 = new_kernel_task( &test_multitasking2 );

	int nn;
	for(nn=0;nn<10;nn++)
	{
		kprintf("task 1, count %d  [%d active tasks]\n", nn, num_tasks());
		yield();
	}	

	kprintf("Waiting for task 2...\n");	
	join(task2);
	kprintf("Task 2 complete, ran %dx\n", task2->return_val);
}

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


	test_multitasking1();

	while(1);
}
