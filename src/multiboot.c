#include "multiboot.h"
#include "kprintf.h"

size_t memory_table(multiboot_info_t* mbd, unsigned int magic)
{
	if(magic != MULTIBOOT_BOOTLOADER_MAGIC)
		return 0;

	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;
    size_t available_mem = 0;
	for(unsigned int nn=0; nn<mbd->mmap_length; nn++)
	{
		if(mmap_table[nn].len>0 && mmap_table[nn].size>0)
        {
            if(mmap_table[nn].type == 1)
            available_mem += mmap_table[nn].len;
        }
		
	}

    return available_mem;
}

int print_memory_table(multiboot_info_t* mbd)
{
	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;
	kprintf("Memory Table:\n");
	for(unsigned int nn=0; nn<mbd->mmap_length; nn++)
	{
        if(mmap_table[nn].size == 0)
            break;

		if(mmap_table[nn].len>0)
			kprintf("%.8llp - %.8llp (%6llu kiB) type: %u\n", mmap_table[nn].addr, mmap_table[nn].addr+mmap_table[nn].len - 1, mmap_table[nn].len/1024, mmap_table[nn].type);
		
	}#  .code16gcc



    // page_directory_t* pd = getPageDirectory();
	// kprintf("Page Directory at %p:\n", pd);
	// for(unsigned int nn=0; nn<64; nn++)
	// {
    //     void* addr = pd[nn] & 0xfffff000;
    //     unsigned int flags = pd[nn] & 0xfff;
    //     if(flags & 1)   // present
    //         kprintf("[%3u] %p (%#x)\n", nn, addr, flags);		
	// }

    // print_crs();

    return 0;
}
