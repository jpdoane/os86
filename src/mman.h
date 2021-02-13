#ifndef __MEMMAN_H__
#define __MEMMAN_H__

#include "multiboot.h"
#include "memory.h"
#include "paging.h"
#include "task.h"

int init_kernel_memory(multiboot_info_t* mbd);
int map_hardware_buffer(void* buffer_virtual, void* buffer_physical, size_t buffer_size, uint32_t page_flags);
int unmap_lowmem();
void kheap_init();
// void kstack_init();
// int relocate_stack(void* old_stack_base, void* new_stack_base);


// increment kernel heap size, allocating new pages
// return pointer to *old* break (beginning of newly allocated memory)
void* ksbrk(size_t increment);
// set end of kernel heap to addr, returns 0 on success
int kbrk(void* addr);

// allocate a new task control block and initialize kernel stack(s) 
task_control_block_t* new_tcb();
// free an existing task control block 
int free_tcb(task_control_block_t* tcb);


uint32_t unmap_page(page_t* page);
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