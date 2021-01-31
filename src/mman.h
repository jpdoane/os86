#ifndef __MEMMAN_H__
#define __MEMMAN_H__

#include "multiboot.h"
#include "paging.h"

int global_memory_init(multiboot_info_t* mbd);

int process_memory_init();

int unmap_lowmem();
void init_vga_buffer();
void kheap_init();
void kstack_init();

// increment kernel heap size, return pointer to *old* break (beginning of newly allocated memory)
void* ksbrk(size_t increment);
// set end of kernel heap to addr, returns 0 on success
int kbrk(void* addr);


int unmap_page(page_t* page);
page_t* map_page(page_t* page, uint32_t flags);
page_t* map_page_at(page_t* page, page_t* frame, uint32_t flags);
page_table_t* new_page_table(void* addr, uint32_t flags);

//return new pointer with offset bytes 
static inline void* ptr_offset(void* p, size_t byte_offset)
{
    return (char*)p + byte_offset;
}


//return unsigned distance between pointers in bytes
static inline uint32_t ptr_diff(void* plow, void* phigh)
{
    return (uint32_t)phigh - (uint32_t)plow;
}

// static inline void* align_to_page(void* addr)
// {
//     return (void*) ((((uint32_t)addr) + PAGE_SIZE - 1) & PAGE_ADDRMASK);
// }


#endif