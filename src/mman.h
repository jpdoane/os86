#ifndef __MEMMAN_H__
#define __MEMMAN_H__

#include "multiboot.h"
#include "paging.h"

int memory_init(multiboot_info_t* mbd);
int unmap_lowmem();
char* map_vga();
void kheap_init();
void kstack_init();

// increment kernel heap size, return pointer to *old* break (beginning of newly allocated memory)
void* ksbrk(size_t increment);
// set end of kernel heap to addr, returns 0 on success
int kbrk(void* addr);


int unmap_page(page_t* page_virt);
page_t* map_page(page_t* page_virt, uint32_t flags);
page_t* map_page_at(page_t* page_virt, page_t* page_phys, uint32_t flags);
page_table_t* new_page_table(void* addr, uint32_t flags);

static inline void* align_addr(void* addr, uint32_t alignment)
{
    uint32_t am = alignment - 1;
    return (char*) (((uint32_t)addr + am) & ~am);
}

static inline int is_pow_of_two(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

// static inline void* align_to_page(void* addr)
// {
//     return (void*) ((((uint32_t)addr) + PAGE_SIZE - 1) & PAGE_ADDRMASK);
// }


#endif