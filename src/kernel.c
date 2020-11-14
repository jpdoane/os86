#include "terminal.h"
#include "kprintf.h"
#include "paging.h"
#include "multiboot.h"
#include "mman.h"

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

	print_memory_table(mbd);
	memory_init(mbd);	//after this the multiboot structure is unmapped

	kprintf("Allocate some memory on the heap...\n");
	uint32_t* some_memory = kmalloc(sizeof(uint32_t)*4000);
	kprintf("setting [0x%x] = 0x%x\n", some_memory + 0, 0x12345678);
	kprintf("setting [0x%x] = 0x%x\n", some_memory + 100, 0xdeadbeef);
	kprintf("setting [0x%x] = 0x%x\n", some_memory + 3000, 0xbeefcafe);
	kprintf("setting [0x%x] = 0x%x\n", some_memory + 3999, 0x1234abcd);

	some_memory[0] = 0x12345678;
	some_memory[100] = 0xdeadbeef;
	some_memory[3000] = 0xbeefcafe;
	some_memory[3999] = 0x1234abcd;

	kprintf("check [0x%x] = 0x%x\n", some_memory + 0, some_memory[0]);
	kprintf("check [0x%x] = 0x%x\n", some_memory + 100, some_memory[100]);
	kprintf("check [0x%x] = 0x%x\n", some_memory + 3000, some_memory[3000]);
	kprintf("check [0x%x] = 0x%x\n", some_memory + 3999, some_memory[3999]);


	while(1);
}
