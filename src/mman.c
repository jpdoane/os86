#include "mman.h"
#include "kprintf.h"

extern char _kernel_end;      //virtual address of end of static kernel space
extern char _kernel_end_phys; //phyiscal address of end of static kernel space

page_t* kstack_base;  // fixed at top of memory
page_t* kstack_brk;   // grows down vvvvvv

page_t* kheap_brk;    // grows up ^^^^^^
page_t* kheap_base;   // just above page stack

// manage free frames in physical memory with a stack
// each stack entry is pointer to physical frame
page_t** frame_stack_start;  //beginning of stack memory, just after _kernel_end
page_t** frame_stack_top;    //current top of stack 
page_t** frame_stack_base;   //end of stack data
page_t*  frame_stack_brk;   //end of allocated memory for page allocation stack

// Kernel space:
// 0xC0000000 - [_kernel_end]:              static kernel space
// [frame_stack_start] - [frame_stack_brk]: frame stack
// [kheap_base] - [kheap_brk]:              kernel heap
// [kheap_brk] - [kstack_brk]:              unused space that heap and stack can grow into
// [kstack_brk] - [kstack_base]:            kernel stack
// 0xffc00000-0xffffffff:                   page tables


// allocate a physical page from the stack
page_t* pop_free_frame()
{
    page_t* free_frame = *(frame_stack_top++);
    if(frame_stack_top<frame_stack_base)
        return free_frame;
    else
        return NULL; //out of memory!    
}

// free a physical page back to the stack
int push_free_frame(page_t* free_frame)
{
    if(frame_stack_top<=frame_stack_start)
        return -1;

    *(frame_stack_top--) = free_frame;
    return 0;
}

// set up a stack will all free pages of physical memory
void init_frame_stack(multiboot_info_t* mbd)
{
	multiboot_memory_map_t* mmap_table = (multiboot_memory_map_t*) mbd->mmap_addr;

    // We will start looking for free physical memory just above the kernel
    page_t* first_free_frame = align_ptr( &_kernel_end_phys, PAGE_SIZE); //physical address of first free page after the kernel

    // Map the stack just past the end of the kernel in virtual mem

    // stack_break points to the end of the allocated region for the stack  
    frame_stack_brk = align_ptr( &_kernel_end, PAGE_SIZE);

    // initialize stack pointers
    // we will initialize the stack by adding new pages to the base
    // first pages (in low mem) will be at top of the stack
    frame_stack_start = (page_t**) frame_stack_brk;
    frame_stack_top = frame_stack_start;
    frame_stack_base = frame_stack_top;

    // allocate one page for the stack, incrementing the stack_break and first_free_frame pointers
    // We will allocate more pages for the stack as we go as needed
    if(!map_page_at(frame_stack_brk++, first_free_frame++, PAGE_FLAG_WRITE))
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
            page_t* pg = align_ptr( (char*) block_addr, PAGE_SIZE); // first whole page in block
            page_t* pg_end = (page_t*) ((block_addr+block_len) & PAGE_ADDRMASK); // first page *after* last whole page in block

            //don't map any memory before end of the kernel
            pg = pg<first_free_frame ? first_free_frame : pg; 

            //fill stack with pointers to each page in the block
            for(; pg<pg_end; pg++)
            {
                // extend end of stack and make sure we have allocated enough memory
                if( (void*) ++frame_stack_base >= (void*) frame_stack_brk)
                {
                    // map a new page at the end of the stack
                    // note that this will pop a free page off the top of the stack
                    if(!map_page(frame_stack_brk++, PAGE_FLAG_WRITE))
                        panic("Error reserving memory for physical page allocation!");
                }

                //record physical page at end stack
                *frame_stack_base = pg;
            }
        }
	}

    // size_t free_frames = frame_stack_base-frame_stack_top;
	// kprintf("\nFound %d frames of RAM (%f MB)\n", free_frames, free_frames*PAGE_SIZE/1e6);

}



// increment heap size, return pointer to *old* break (beginning of newly allocated memory)
void* ksbrk(size_t increment)
{
    if(increment == 0) return kheap_brk;

    page_t* old_brk = kheap_brk;

    if(kbrk(((char*)kheap_brk) + increment)) return NULL;

    return old_brk;
}

// set kheap break to addr, growing or shrinking heap accordingly
// returns 0 on success
int kbrk(void* addr)
{
    //make sure this is a valid break (not before heap begins or into stack)
    if(addr < (void*) kheap_base || addr >= (void*) kstack_brk)
        return -1; 

    page_t* pg_addr = align_ptr(addr, PAGE_SIZE); // align addr to a page

    if(pg_addr > kheap_brk)
    {
        // we are increasing size of the heap
        while(pg_addr > kheap_brk)
        {
            // allocate a new page and map to end of heap
            if(!map_page(kheap_brk++, PAGE_FLAG_WRITE))
                return -1; // out of physical memory
        }
    }
    else
    {
        // we are decreasing size of the heap
        while(pg_addr < kheap_brk)
        {
            //free page from end of heap
            if(unmap_page(--kheap_brk))
                return -1; // error!
        }
    }

	// kprintf("Setting kernel heap break to 0x%x\n", kheap_brk);
    
    return 0;
}


int global_memory_init(multiboot_info_t* mbd)
{
    init_frame_stack(mbd);

    // define some dummy pointers for where we will put the stack
    // TODO: actually set up a stack here
    kstack_base = (page_t*) 0xffb00000;
    kstack_brk = (page_t*)  0xf0000000;

    kheap_base = frame_stack_brk; //start heap after the page stack allocator
    kheap_brk = kheap_base;     // heap starts empty - increase with ksbrk

    // we don't need the 1st MB anymore so unmap it
	if( unmap_lowmem() )
        return -1;

    // except for VGA memory - still need that..
	// if(! map_vga() )
    //     return -1;

    return 0;
}

// int process_memory_init()
// {
//     // define some dummy pointers for where we will put the stack
//     // TODO: actually set up a stack here
//     kstack_base = (page_t*) 0xffb00000;
//     kstack_brk = (page_t*)  0xf0000000;

//     return 0;
// }


int unmap_lowmem()
{
    pd->dir.pde[0] = PAGE_FLAG_EMPTY;
    refresh_tlb();
    return 0;
}

// char* map_multiboot()
// {

// }


//map vga buffer memory
void init_vga_buffer()
{
    page_t* vga_buf = (page_t*) VGA_BUFFER;
    page_t* vga_buf_phys = (page_t*) VGA_BUFFER_PHYS;

    size_t Npages = VGA_BUFFER_SIZE >> 12;
    for(size_t nn = 0; nn < Npages; nn++)
        map_page_at(vga_buf++,vga_buf_phys++, PAGE_FLAG_USER | PAGE_FLAG_WRITE);
}


int map_hardware_buffer(void* buffer_virtual, void* buffer_physical, size_t buffer_size, uint32_t page_flags)
{
    //ensure buffers are page-aligned
    if(((size_t) buffer_virtual & ~PAGE_ADDRMASK) || ((size_t) buffer_physical & ~PAGE_ADDRMASK) )
        return -1;

    //ensure virtual address is in kernel space
    if(buffer_virtual < (void*) KERNEL_BASE)
        return -1;

    page_t* page = (page_t*) buffer_virtual;
    page_t* frame = (page_t*) buffer_physical;

    size_t Npages = (buffer_size + PAGE_SIZE - 1) >> 12;
    for(size_t nn = 0; nn < Npages; nn++)
        if(!map_page_at(page++,frame++, page_flags)) return -1;
    
    return 0;
}


page_t* map_page_at(page_t* page, page_t* frame, uint32_t flags)
{
    page_table_t* pt = get_table(page);

    if(!pt)
    {
        // we are setting up all the page tables for kernel space (>0xc0000000) at boot,
        // so if a PT missing theres a problem
        panic("Kernel page table missing!");
        // pt = new_page_table(page, flags); // page table is not present, allocate a new one
    }
    
    if(pt->pte[get_ptindex(page)] & PAGE_FLAG_PRESENT)
        return NULL; //page already mapped

    // map page
    pt->pte[get_ptindex(page)] = (((uint32_t) frame) & PAGE_ADDRMASK) | PAGE_FLAG_PRESENT | flags;

    refresh_page(page);
    return page;
}

page_t* map_page(page_t* page, uint32_t flags)
{
    return map_page_at(page, pop_free_frame(), flags);
}

int unmap_page(page_t* page)
{
    return push_free_frame(get_physaddr(page));
}

page_table_t* new_page_table(void* addr, uint32_t flags)
{
    uint32_t pdindex = get_pdindex(addr);      // index into page dir (which table does this address point to)

    // is this table already mapped?
    if(pd->dir.pde[pdindex] & PAGE_FLAG_PRESENT)
        return NULL;

    page_t* pp = pop_free_frame();         // find a free page in physical mem for page table
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


// void map_kernel_mem()
// {
//     for(int pdindex = KMIN; pdindex = KMAX; pdindex++)
//         pd->dir.pde[pdindex] = (((uint32_t) pop_free_frame()) & PAGE_ADDRMASK) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
// }
