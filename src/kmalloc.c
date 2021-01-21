#include "kmalloc.h"
#include "mman.h"

void* kheap = NULL;

void* kmalloc_aligned(size_t sz, size_t alignment)
{
    if( sz == 0 || alignment < sizeof(size_t) || !is_pow_of_two(alignment) ) return NULL;

    if(!kheap)
    {
        kheap = ksbrk(0);
        if(!kheap) return NULL;
    }

    // just keep track of pointer at end of allocated heap, and add new allocations on end
    // note - this won't let us free!
    void* mem = align_addr(kheap, alignment); //start of requested region
    void* mem_end = align_addr(mem+sz, alignment); //end of requested region
    
    // if this allocation exceeds the current heap, request more heap
    void* kheap_brk = ksbrk(0);
    if(mem_end > kheap_brk)
        if(kbrk(mem_end))
            return NULL;

    kheap = mem_end;    //update heap pointer;

    return mem;
}


void* kmalloc(size_t sz)
{
    return kmalloc_aligned(sz, 4);
}
