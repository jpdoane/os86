#ifndef __PAGING_H__
#define __PAGING_H__

#include "multiboot.h"
#include "memory.h"

#define PAGE_FLAG_EMPTY              0
#define PAGE_FLAG_PRESENT            1
#define PAGE_FLAG_WRITE              1<<1
#define PAGE_FLAG_USER               1<<2
#define PAGE_FLAG_WRITETHROUGH       1<<3
#define PAGE_FLAG_CACHEDISABLED      1<<4
#define PAGE_FLAG_ACCESSED           1<<5
#define PAGE_FLAG_DIRTY              1<<6
#define PAGE_FLAG_SIZE               1<<7
#define PAGE_FLAG_PATTYPE            1<<7
#define PAGE_FLAG_GLOBAL             1<<8
#define PAGE_FLAGSMASK          0xfff
#define PAGE_ADDRMASK           0xfffff000
#define PAGE_PTE_MASK           0x003ff000

#define _4KB 0x1000
#define _1MB 0x100000
#define _4MB 0x400000

#define PAGE_SIZE _4KB
#define PAGING_NUM_PDE 0x400        //1024
#define PAGING_NUM_PTE 0x400        //1024

#define ALIGN_TO_PAGE __attribute__ ((aligned (PAGE_SIZE)))

#ifndef ASM_FILE
#include <stdint.h>
#include <stddef.h>

typedef struct page_t {
    uint32_t word[PAGE_SIZE/sizeof(uint32_t)] ALIGN_TO_PAGE;
} page_t;

typedef struct page_directory_t {
    uint32_t pde[PAGING_NUM_PDE] ALIGN_TO_PAGE;
} page_directory_t;

typedef struct page_table_t {
    uint32_t pte[PAGING_NUM_PTE] ALIGN_TO_PAGE;
} page_table_t;

// this is the paging structure in virtual memory after self-mapping the last PDE
// accessing virtual memory at 0xffcxxxxx will select the page dir itself as its own page table
// then the middle bits will select the offset into the directory, and return the page table as the page
// There are therefore 1023 page tables, with the final table being the directory itself. 
typedef struct paging_t {
    page_table_t tables[PAGING_NUM_PDE-1] ALIGN_TO_PAGE;
    page_directory_t dir ALIGN_TO_PAGE;
} paging_t;


//global pointer to page directory
extern paging_t* paging;

// return page table index for virtual address
static inline uint32_t get_ptindex(void* addr)
{
    return (((uint32_t) addr) & PAGE_PTE_MASK ) >> 12;
}

// return page directory index for virtual address
static inline uint32_t get_pdindex(void* addr)
{
    return ((uint32_t) addr) >> 22;
}

// return offset into page
static inline uint32_t get_page_offset(void* addr)
{
    return ((uint32_t) addr) & ~PAGE_ADDRMASK;
}


static inline void* get_virtual_addr(uint32_t pd_index, uint32_t pt_index, uint32_t offset)
{
    return (void*) (pd_index << 22 | pt_index << 12 | offset);
}

//asm functions defined in paging.s
void enable_paging(page_directory_t* pd);
void set_page_dir(page_directory_t* pd);
page_directory_t* get_page_dir();
void refresh_page(void* addr);
void refresh_tlb();

// c functions defined in paging.c
page_table_t* get_table(void* addr);
void* get_physaddr(void* addr); // return physical addr from virt addr

#endif


#endif