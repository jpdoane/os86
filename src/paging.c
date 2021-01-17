#include "paging.h"

page_directory_virt_t* pd = (page_directory_virt_t*) PD_ADDR;

// return page directory index for virtual address
page_table_t* get_table(void* addr)
{
    uint32_t idx = get_pdindex(addr);
    if(pd->dir.pde[idx] & PAGE_FLAG_PRESENT)
        return pd->tables + get_pdindex(addr);
    else 
        return NULL;
}

// return physical addr from virt addr
void* get_physaddr(void* addr)
{
    //check pd entry is present
    uint32_t pdi = get_pdindex(addr);
    if(! pd->dir.pde[pdi] & PAGE_FLAG_PRESENT)
        return NULL;

    //check pt entry is present
    uint32_t pte = pd->tables[pdi].pte[get_ptindex(addr)];
    if(! pte & PAGE_FLAG_PRESENT)
        return NULL;

    return (void*) ((pte & PAGE_ADDRMASK) +  get_page_offset(addr) );
}
