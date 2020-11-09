#include "paging.h"
#include "kprintf.h"

// map a continuous region of physical memory to page table starting at addr [base] and table entry [pte]
// maps [num_pages] pages until/unless table fills up
// returns number of 4kB pages mapped
size_t map_pages(page_table_t* pte, addr_t base, size_t num_pages)
{
    size_t nn;
    for(nn = 0; nn < num_pages; nn++)
    {
        *pte = base | PTE_PRESENT;
        base += _4KB;
        pte++;

        //if pte addr is page aligned, the pt is full
        if( !((addr_t)pte & 0xfff) )
            break;
    }
    return nn;
}

void init_page_dir(page_directory_t* pd)
{
    set_page_dir(pd);

    //initialize empty page directory
    for(int nn=0; nn<PAGING_NUM_PDE-1; nn++)
        pd[nn] = PDE_EMPTY;

    //map last pde to itself
    //PD will be accessible at virtual addr 0xfffffxxx 
    pd[PAGING_NUM_PDE-1] =  ((addr_t) pd) & PDE_ADDRMASK + PDE_PRESENT;
}


// set up paging
// set up first table for identify mapping of 1st 1MB
// set up second table for 1st MB of kernel memory
// returns 0 on success
int initialize_paging()
{
    page_directory_t* pd = (page_directory_t*) PAGING_PD_KERN;
    init_page_dir(pd);

    //set up identity page table for 1st 1MB
    pd[0] = PAGING_PT_1MB | PDE_PRESENT;
    map_pages(PAGING_PT_1MB, 0x0, PAGING_PT_SIZE);

    //set up 1MB of kernel-space memory
    int kern_base_pde = KERNEL_BASE >> 22;
    pd[kern_base_pde] = PAGING_PT_KERN | PDE_PRESENT;
    map_pages(PAGING_PT_KERN, KERNEL_BASE_PHYS, PAGING_PT_SIZE);

    enable_paging(pd); //asm call

    return 0;
}
 

// return physical addr from virt addr
// virtual addr is:
// [31-22] page directory index (10 bits, 0-1023)
// [21-12] page table index (10 bits, 0-1023)
// [11-0]  offset into page (12 bits, 0-4095)
// populates virt_addr and page table entry flags
// returns 0 on success, -1 on page miss
int get_physaddr(addr_t* phys_addr, addr_t virt_addr, page_directory_t* pd, int* pte_flags)
{
    unsigned int pdindex = virt_addr >> 22;
    unsigned int ptindex = virt_addr >> 12 & 0x03FF;
    unsigned int offset = virt_addr & 0xFFF;

    //check pd entry is present
    if(! pd[pdindex] & PDE_PRESENT)
        return -1;

    page_table_t* pt = (page_table_t*) (((addr_t)pd[pdindex]) & PDE_ADDRMASK);

    //check pt entry is present
    if(! pt[ptindex] & PTE_PRESENT)
        return -1;

    *pte_flags = pt[ptindex] & PTE_FLAGSMASK; 
    *phys_addr = pt[ptindex] & PTE_ADDRMASK + virt_addr & 0xFFF;
    return 0;
}


void print_crs()
{
    unsigned int cr0, cr2, cr3, cr4;
    __asm__ __volatile__ (
        "mov %%cr0, %%eax\n"
        "mov %%eax, %0\n"
        "mov %%cr2, %%eax\n"
        "mov %%eax, %1\n"
        "mov %%cr3, %%eax\n"
        "mov %%eax, %2\n"
        "mov %%cr4, %%eax\n"
        "mov %%eax, %2\n"
    : "=m" (cr0), "=m" (cr2), "=m" (cr3), "=m" (cr4)
    : /* no input */
    : "%eax"
    );
    
    kprintf("cr0: %#x\n", cr0);
    kprintf("cr2: %#x\n", cr2);
    kprintf("cr3: %#x\n", cr3);
    kprintf("cr4: %#x\n", cr4);
}
