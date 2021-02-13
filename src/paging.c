#include "paging.h"

paging_t* paging = (paging_t*) PAGING_ADDR;

// return page directory index for virtual address
page_table_t* get_table(void* addr)
{
    uint32_t idx = get_pdindex(addr);
    if(paging->dir.pde[idx] & PAGE_FLAG_PRESENT)
        return paging->tables + get_pdindex(addr);
    else 
        return NULL;
}

// return physical addr from virt addr
void* get_physaddr(void* addr)
{
    //check paging entry is present
    uint32_t pdi = get_pdindex(addr);
    if(! paging->dir.pde[pdi] & PAGE_FLAG_PRESENT)
        return NULL;

    //check pt entry is present
    uint32_t pte = paging->tables[pdi].pte[get_ptindex(addr)];
    if(! pte & PAGE_FLAG_PRESENT)
        return NULL;

    return (void*) ((pte & PAGE_ADDRMASK) +  get_page_offset(addr) );
}
