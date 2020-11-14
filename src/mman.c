#include "mman.h"
#include "kprintf.h"

extern char _kernel_end_phys; //address of this points to end of kernel space and start of available memeory

page_t* heap_page;      
page_t* heap_page_phys;

char* heap;
char* heap_end;
char* heap_end_phys;

int memory_init(multiboot_info_t* mbd)
{
	if(! heap_init(mbd) )
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

char* heap_init(multiboot_info_t* mbd)
{
    char* heap_phys;
	size_t free_mem_size = get_big_block(mbd, (void**) &heap_phys);
	heap_end_phys = heap_phys+free_mem_size;

	// this memory is not actually completely free, but includes the kernel
	if(&_kernel_end_phys >= heap_phys && &_kernel_end_phys < heap_end_phys)
		heap_phys = &_kernel_end_phys;

    //start new allocations here in physical memory
    heap_page_phys = (page_t*) align_addr((void*) heap_phys, PAGE_SIZE);
    heap_phys = (char*) heap_page_phys;

    //start new allocations here in virtual memory
    heap_page = (page_t*) align_addr((void*) KERNEL_HEAP, PAGE_SIZE);
    heap = (char*) heap_page;
    heap_end = (char*) KERNEL_BASE;

	kprintf("\nPhysical heap: 0x%.8x - 0x%.8x (%u MiB)\n", heap_phys, heap_end_phys,  (uint32_t) (heap_end_phys-heap_phys) >> 20);
	kprintf("Virtual heap: 0x%.8x - 0x%.8x (%u MiB)\n\n", heap, heap_end,  (uint32_t) (heap_end-heap) >> 20);

    //map the first page on the heap
    if(!map_page_at(heap_page, heap_page_phys, PAGE_FLAG_WRITE))
        return NULL;

    return heap;
}


size_t get_big_block(multiboot_info_t* mbd, void** block)
{
	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;
    size_t block_size = 0;
	for(unsigned int nn=0; nn<mbd->mmap_length; nn++)
	{
        if(mmap_table[nn].size == 0)
            break;

		if(mmap_table[nn].len>block_size && mmap_table[nn].type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            block_size = mmap_table[nn].len;
            if(mmap_table[nn].addr >> 32)
            {
                // ignore anything above 32bits
                break;
            }
            *block = (void*) (uint32_t) mmap_table[nn].addr;
        }
	}
    return block_size;
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

page_table_t* new_page_table(void* addr, uint32_t flags)
{
    uint32_t pdindex = get_pdindex(addr);      // index into page dir (which table does this address point to)

    // is this table already mapped?
    if(pd->dir.pde[pdindex] & PAGE_FLAG_PRESENT)
        return NULL;

    page_t* pp = get_next_heap_page_phys();         // find a free page in physical mem for page table
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
        page_t* pp = get_next_heap_page_phys();    // find a free page in physical mem
        page_t* pv = get_next_heap_page_virt();         // find a free page in virtual mem

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

// return next available physical page from the heap and updates the heap_page_phys pointer
// returns NULL on OOM
page_t* get_next_heap_page_phys()
{
    if( (uint32_t) (heap_page_phys+1) > (uint32_t) heap_end_phys)
    {
        //out of memory
        return NULL;
    }
    return ++heap_page_phys;
}

// returns next available virtual page from the heap and updates the heap_page_phys pointer
// this does *not* update the heap pointer
// returns NULL on OOM
page_t* get_next_heap_page_virt()
{
    if( (uint32_t) (heap_page+1) > (uint32_t) heap_end)
    {
        //out of memory
        return NULL;
    }
    return ++heap_page;
}
