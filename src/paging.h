#ifndef __MEMMAN_H__
#define __MEMMAN_H__

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

/******* physical memory layout *************
0x00000000  0x000fffff   1MB
0x00100000  0x001fffff   kernel space
0x00200000  MAX          user space
********************************************/

/******* virtual memory layout, kernel *************
0x00000000  0xbfffffff   user space
0xc0000000               kernel space
0xc0010000               page tables
****************************************************/

// these must be consistent with linker script
#define KERNEL_BASE_PHYS    0x00101000
#define KERNEL_BASE         0xc0000000

#define KERNEL_VIRT_TO_PHYS(p) p-KERNEL_BASE+KERNEL_BASE_PHYS
#define KERNEL_PHYS_TO_VIRT(p) p+KERNEL_BASE-KERNEL_BASE_PHYS

#define PAGING_NUM_PDE 0x400        //1024
#define PAGING_PD_SIZE 0x1000       //4KB

#define PAGING_NUM_PTE 0x400        //1024
#define PAGING_PT_SIZE 0x1000       //4KB

#define PDE_EMPTY           0
#define PDE_PRESENT         1
#define PDE_READWRITE       1<<1
#define PDE_USERSUPERVISOR  1<<2
#define PDE_WRITETHROUGH    1<<3
#define PDE_CACHEDISABLED   1<<4
#define PDE_ACCESSED        1<<5
#define PDE_DIRTY           1<<6
#define PDE_PATTYPE         1<<7
#define PDE_GLOBAL          1<<8
#define PDE_FLAGSMASK        0xfff
#define PDE_ADDRMASK        0xfffff000

#define PTE_EMPTY           0
#define PTE_PRESENT         1
#define PTE_READWRITE       1<<1
#define PTE_USERSUPERVISOR  1<<2
#define PTE_WRITETHROUGH    1<<3
#define PTE_CACHEDISABLED   1<<4
#define PTE_ACCESSED        1<<5
#define PTE_SIZE            1<<6
#define PTE_FLAGSMASK        0xfff
#define PTE_ADDRMASK        0xfffff000

#define PAGING_PD_KERN 0x1000       // addr of kernel's page dir
#define PAGING_PT_1MB  0x2000       // addr of identity page table for 1st 1MB
#define PAGING_PT_KERN 0x3000       // addr of kernel's 1st page table

#define _4KB 0x1000
#define _1MB 0x100000
#define _4MB 0x400000


#ifndef ASM_FILE
#include <stdint.h>
#include <stddef.h>

typedef uint32_t addr_t;
// typedef uint32_t page_directory_t;
// typedef uint32_t page_table_t;

typedef struct page_directory_t {
    uint32_t pde[PAGING_NUM_PDE];
} page_directory_t;

typedef struct page_table_t {
    uint32_t pte[PAGING_NUM_PTE];
} page_table_t;


//asm functions defined in paging.s
void enable_paging(page_directory_t* pd);
void set_page_dir(page_directory_t* pd);
page_directory_t* get_page_dir();

// c functions defined in paging.c
size_t map_pages(page_table_t* pte, addr_t base, size_t num_pages);
void init_page_dir(page_directory_t* pd);
int initialize_paging();
int get_physaddr(addr_t* phys_addr, addr_t virt_addr, page_directory_t* pd, int* pte_flags);
void print_crs();

#endif


#endif