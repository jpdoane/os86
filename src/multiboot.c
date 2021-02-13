#include "multiboot.h"
#include "kprintf.h"

size_t memory_table(multiboot_info_t* mbd)
{
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
		{
			if(mmap_table[nn].type == MULTIBOOT_MEMORY_AVAILABLE)
				kprintf("%.8llp - %.8llp [AVAILABLE] (%6llu kiB)\n", mmap_table[nn].addr, mmap_table[nn].addr+mmap_table[nn].len - 1, mmap_table[nn].len/1024);
			else
				kprintf("%.8llp - %.8llp [RESERVED]\n", mmap_table[nn].addr, mmap_table[nn].addr+mmap_table[nn].len - 1, mmap_table[nn].len/1024);
		}
		
	}

    return 0;
}
