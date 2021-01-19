#include "mman.h"
#include "kprintf.h"


page_t* heap_page;      
char* heap;
char* heap_end;


// physical page allocation
extern char _kernel_end;      //virtual address of end of static kernel space
extern char _kernel_end_phys; //phyiscal address of end of static kernel space

// manage all free physical pages with a stack
// each stack entry is pointer to physical page
page_t** free_stack_start;  //beginning of stack memory
page_t** free_stack_top;    //current top of stack (free_stack_start <= free_stack_top <= free_stack_base)
page_t** free_stack_base;   //end of stack data
page_t*  free_stack_break;   //end of allocated memory for stack

// allocate a physical page from the stack
page_t* pop_free_page()
{
    page_t* free_page = *(free_stack_top++);
    if(free_stack_top<free_stack_base)
        return free_page;
    else
        return NULL; //out of memory!    
}

// free a physical page back to the stack
int push_free_page(page_t* free_page)
{
    if(free_stack_top<=free_stack_start)
        return -1;

    *(free_stack_top--) = free_page;
    return 0;
}


// set up a stack will all free pages of physical memory
void init_page_stack(multiboot_info_t* mbd)
{
	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;

    // We will start looking for free physical memory just above the kernel
    page_t* first_free_page = align_addr( &_kernel_end_phys, PAGE_SIZE); //physical address of first free page after the kernel

    // Map the stack just past the end of the kernel in virtual mem

    // stack_break points to the end of the allocated region for the stack  
    free_stack_break = align_addr( &_kernel_end, PAGE_SIZE);

    // initialize stack pointers
    // we will initialize the stack by adding new pages to the base
    // first pages (in low mem) will be at top of the stack
    free_stack_start = (page_t**) free_stack_break;
    free_stack_top = free_stack_start;
    free_stack_base = free_stack_top;

    // allocate one page for the stack, incrementing the stack_break and first_free_page pointers
    // We will allocate more pages for the stack as we go as needed
    if(!map_page_at(free_stack_break++, first_free_page++, PAGE_FLAG_WRITE))
        panic("Error reserving memory for physical page allocation!");

    // loop through each block of memory defined in the multiboot record
    // for each block of memory, add free pages to stack
	for(unsigned int nn=0; nn<mbd->mmap_length; nn++)
	{
        if(mmap_table[nn].size == 0) // end of table
            break;

        if(mmap_table[nn].addr >> 32) // ignore anything above 32bits
            continue;

		if(mmap_table[nn].type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            //pointers to the first and last page in this memory block 
            size_t block_addr = mmap_table[nn].addr;
            size_t block_len = mmap_table[nn].len;
            page_t* pg = align_addr( (char*) block_addr, PAGE_SIZE); // first whole page in block
            page_t* pg_end = (page_t*) ((block_addr+block_len) & PAGE_ADDRMASK); // first page *after* last whole page in block

            //don't map any memory before end of the kernel
            pg = pg<first_free_page ? first_free_page : pg; 

            //fill stack with pointers to each page in the block
            for(; pg<pg_end; pg++)
            {
                // extend end of stack and make sure we have allocated enough memory
                if( (void*) ++free_stack_base >= (void*) free_stack_break)
                {
                    // map a new page at the end of the stack
                    // note that this will pop a free page off the top of the stack
                    if(!map_page(free_stack_break++, PAGE_FLAG_WRITE))
                        panic("Error reserving memory for physical page allocation!");
                }

                //record physical page at end stack
                *free_stack_base = pg;
            }
        }
	}

    size_t free_pages = free_stack_base-free_stack_top;
	kprintf("\nFound %d pages (%f MB)\n", free_pages, free_pages*PAGE_SIZE/1e6);

}

int memory_init(multiboot_info_t* mbd)
{
    init_page_stack(mbd);

	if(! heap_init() )
        return -1;

	if( unmap_lowmem() )
        return -1;

	// if(! map_multiboot() )
    //     return -1;

	if(! map_vga() )
        return -1;

    return 0;
}

int unmap_lowmem()
{
    pd->dir.pde[0] = PAGE_FLAG_EMPTY;
    refresh_tlb();
    return 0;
}

// char* map_multiboot()
// {

// }

char* map_vga()
{
    page_t* vga_buf = (page_t*) VGA_BUFFER;

    //map 6 pages of vga memory    
    map_page_at(vga_buf,vga_buf, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+1,vga_buf+1, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+2,vga_buf+2, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+3,vga_buf+3, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+4,vga_buf+4, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+5,vga_buf+5, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    map_page_at(vga_buf+6,vga_buf+6, PAGE_FLAG_USER | PAGE_FLAG_WRITE);

    return (char*) vga_buf;
}

char* heap_init()
{
    //start new allocations here in virtual memory
    heap_page = (page_t*) align_addr((void*) KERNEL_HEAP, PAGE_SIZE);
    heap = (char*) heap_page;
    heap_end = (char*) KERNEL_HEAP_END;

	kprintf("Setting up heap: 0x%.8x - 0x%.8x (%u MiB)\n\n", heap, heap_end,  (uint32_t) (heap_end-heap) >> 20);

    //map the first page on the heap
    if(!map_page(heap_page, PAGE_FLAG_WRITE))
        return NULL;

    return heap;
}


page_t* map_page_at(page_t* page_virt, page_t* page_phys, uint32_t flags)
{
    page_table_t* pt = get_table(page_virt);

    if(!pt)
        pt = new_page_table(page_virt, flags); // page table is not present, allocate a new one
    
    if(pt->pte[get_ptindex(page_virt)] & PAGE_FLAG_PRESENT)
        return NULL; //page already mapped

    // map page
    pt->pte[get_ptindex(page_virt)] = (((uint32_t) page_phys) & PAGE_ADDRMASK) | PAGE_FLAG_PRESENT | flags;

    refresh_page(page_virt);
    return page_virt;
}

page_t* map_page(page_t* page_virt, uint32_t flags)
{
    return map_page_at(page_virt, pop_free_page(), flags);
}


page_table_t* new_page_table(void* addr, uint32_t flags)
{
    uint32_t pdindex = get_pdindex(addr);      // index into page dir (which table does this address point to)

    // is this table already mapped?
    if(pd->dir.pde[pdindex] & PAGE_FLAG_PRESENT)
        return NULL;

    page_t* pp = pop_free_page();         // find a free page in physical mem for page table
    if(!pp) return NULL; // OOM

    // we don't need to allocate virtual memory space, since table lives in our paging structure
    page_table_t* pv = pd->tables + pdindex;        // pointer to table (virtual)

    // map table: point the pd entry at the physical page we just reserved, and set flags
    pd->dir.pde[pdindex] = (((uint32_t) pp) & PAGE_ADDRMASK) | PAGE_FLAG_PRESENT | flags;

    // flush the tlb
    refresh_page(pv);

    //clear table
    memset((char*) pv, 0, PAGE_SIZE);     

    // return pointer to table (virtual)
    return pv;
}

void* kmalloc_aligned(size_t sz, size_t alignment)
{
    if( sz == 0 || !is_pow_of_two(alignment) ) return NULL;

    char* mem = align_addr(heap, alignment); //start of allocated region
    char* mem_end = align_addr(mem+sz, alignment); //first byte past end of allocated region

    if(mem_end > heap_end)
    {
        // out of memory!
        return NULL;
    }

    //map new pages
    page_t* page_end = (page_t*) (((uint32_t) mem_end - 1) & PAGE_ADDRMASK);
    while(page_end > heap_page)
    {
        page_t* pp = pop_free_page();    // find a free page in physical mem
        page_t* pv = get_next_heap_page();         // find a free page in virtual mem

        if(!pp || !pv)
            return NULL; //OOM

        if(!map_page_at(pv, pp, PAGE_FLAG_WRITE))
            return NULL;
    }

    heap = mem_end;     //update pointer to end of used heap
    return (void*) mem;
}


void* kmalloc(size_t sz)
{
    return kmalloc_aligned(sz, 4);
}

void* memset(void* addr, int val, size_t cnt)
{
    char* m = (char*) addr;
    for(size_t nn=0;nn<cnt; nn++)
        m[nn] = val;

    return addr;
}

// returns next available virtual page from the heap and updates the heap_page_phys pointer
// this does *not* update the heap pointer
// returns NULL on OOM
page_t* get_next_heap_page()
{
    if( (uint32_t) (heap_page+1) > (uint32_t) heap_end)
    {
        //out of memory
        return NULL;
    }
    return ++heap_page;
}
