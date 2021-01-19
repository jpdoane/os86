#ifndef __MEMMAN_H__
#define __MEMMAN_H__

#include "multiboot.h"
#include "paging.h"

typedef struct page_t {
    uint32_t word[PAGE_SIZE/4];
} page_t;



int memory_init(multiboot_info_t* mbd);
int unmap_lowmem();
char* map_vga();
char* heap_init();

page_t* map_page(page_t* page_virt, uint32_t flags);
page_t* map_page_at(page_t* page_virt, page_t* page_phys, uint32_t flags);
page_table_t* new_page_table(void* addr, uint32_t flags);

page_t* get_next_heap_page();

void* kmalloc(size_t sz);
void* kmalloc_aligned(size_t sz, size_t alignment);
void* memset(void* addr, int val, size_t cnt);

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