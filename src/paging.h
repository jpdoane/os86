#ifndef __PAGING_H__
#define __PAGING_H__

#include "multiboot.h"

/**** 1st MB layout (identity mapped) ******
0x00000000 	0x000003FF 	Real Mode IVT
0x00000400 	0x000004FF 	BIOS data area (BDA)
0x00000500 	0x00000fff 	AVAILABLE
0x00001000 	0x00001fff 	kernel page directory
0x00002000 	0x00002fff 	Identity page table for 1st MB
0x00003000 	0x00003fff 	kerenel's 1st page table
0x00004000 	0x00007BFF 	AVAILABLE
0x00007C00 	0x00007DFF 	BootSector (512B)
0x00007E00 	0x0007FFFF 	AVAILABLE (480kB)
0x00080000 	0x0009FFFF 	Extended BIOS Data Area
0x000A0000 	0x000BFFFF 	Video display memory
0x000C0000 	0x000C7FFF 	Video BIOS
0x000C8000 	0x000EFFFF 	BIOS Expansions
0x000F0000 	0x000FFFFF 	Motherboard BIOS 
********************************************/


// example paging table layout after boot
// cr3: 0x0010a000                                      Page directory (boot_page_directory)
// 0x00000000-0x003fffff -> 0x00000000-0x003fffff       identity map of 1st 4MiB (uses pd[0])
// 0x00400000-0x00400fff -> 0x0010d000-0x0010dfff       first page allocated on heap: (uses pd[1], pt[0])
// 0xc0000000-0xc000bfff -> 0x00101000-0x0010cfff       kernel pages (uses pd[0x300] = pd+0xc00)
// 0xffc00000-0xffc00fff -> 0x00000000-0x00000fff       pd[0] remaps lowest page (not actual page table but intepreted as one)
// 0xffc01000-0xffc01fff -> 0x0010e000-0x0010efff       pd[1] page table, maps heap pages 0x00400000-0x007ff000
// 0xfff00000-0xfff00fff -> 0x0010b000-0x0010bfff       pd[0x300] (boot_pt_kernel), maps kernel pages 0xc0000000-0xc03ff000
// 0xfffff000-0xffffffff -> 0x0010a000-0x0010afff       Self-map of last pde -> pd

// extern char _kernel_start_phys;
// extern char _kernel_start;
// #define KERNEL_BASE_PHYS    &_kernel_start_phys //0x00101000
// #define KERNEL_BASE         &_kernel_start      //0xc0000000

// these must be consistent with linker script
#define KERNEL_BASE_PHYS    0x00101000
#define USER_CODE           0x01000000
#define USER_STACK_BASE     0xbffffff0
#define KERNEL_BASE         0xc0000000
#define KERNEL_STACK_BASE   0xffb00000

#define KERNEL_STACK_SIZE   32768

#define KERNEL_VIRT_TO_PHYS(p) p-KERNEL_BASE+KERNEL_BASE_PHYS
#define KERNEL_PHYS_TO_VIRT(p) p+KERNEL_BASE-KERNEL_BASE_PHYS

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

#define PD_ADDR         0xffc00000  //virtual address of paging structure

#ifndef ASM_FILE
#include <stdint.h>
#include <stddef.h>

typedef struct page_t {
    uint32_t word[PAGE_SIZE/4];
} page_t;

typedef struct page_directory_t {
    uint32_t pde[PAGING_NUM_PDE];
} page_directory_t;

typedef struct page_table_t {
    uint32_t pte[PAGING_NUM_PTE];
} page_table_t;

// this is the paging structure in virtual memory after self-mapping the last PDE
// accessing virtual memory at 0xffcxxxxx will select the page dir itself as its own page table
// then the middle bits will select the offset into the directory, and return the page table as the page
// There are therefore 1023 page tables, with the final table being the directory itself. 
typedef struct page_directory_virt_t {
    page_table_t tables[PAGING_NUM_PDE-1];
    page_directory_t dir;
} page_directory_virt_t;


//global pointer to page directory
extern page_directory_virt_t* pd;   //initialized in paging_asm.S

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