#include "test.h"
#include "kmalloc.h"
#include "kprintf.h"


//allocate memory, return ptr
//check heap usage before and after for consistenty with expected allocation
int test_allocation(size_t sz, void** ptr)
{
    size_t sz_used;
    size_t sz_free;
    // get initial size of heap
    if(kheap_size(&sz_used, &sz_free)) return -1;

    // allocate memory
    *ptr = kmalloc(sz);
    if(! *ptr) return -1;

    // check heap size
    size_t sz_used_new;
    size_t sz_free_new;
    if(kheap_size(&sz_used_new, &sz_free_new)) return -1;
    // memory is allocated in increments of HEAP_BLOCKSIZE_UNITS, with extra HEAP_BLOCKSIZE_UNITS for header
    size_t sz_alloc = HEAP_BLOCKSIZE_UNITS + align_int(sz, HEAP_BLOCKSIZE_UNITS);

    // allocated memory should have increased by sz_alloc
    if(sz_used_new != sz_used + sz_alloc)
        return -1;

    // total heap size (used + free) should not have decreased
    if(sz_used_new + sz_free_new < sz_used + sz_free)
        return -1;

    // if heap size (used + free) has changed, it must have increased in units of HEAP_INCREMENT 
    if( ( (sz_used_new + sz_free_new) - (sz_used + sz_free) ) % HEAP_INCREMENT != 0)
        return -1;

    return 0;

}

// free memory and check heap for consistenty with expected free
int test_free(size_t sz, void* ptr)
{
    size_t sz_used;
    size_t sz_free;
    // get initial size of heap
    if(kheap_size(&sz_used, &sz_free)) return -1;

    // test free and make sure we have recovered all the memory
    if( kfree(ptr) ) return -1;
    // memory is allocated in increments of HEAP_BLOCKSIZE_UNITS, with extra HEAP_BLOCKSIZE_UNITS for header
    size_t sz_alloc = HEAP_BLOCKSIZE_UNITS + align_int(sz, HEAP_BLOCKSIZE_UNITS);

    size_t sz_used_new;
    size_t sz_free_new;
    if(kheap_size(&sz_used_new, &sz_free_new)) return -1;

    if(sz_used_new != sz_used - sz_alloc)
        return -1;
    if(sz_free_new != sz_free + sz_alloc)
        return -1;

    return 0;
}

int test_kmalloc()
{
    if(kmalloc_init()) return -1;

    //allocate uint32_t
    uint32_t* a;
    if(test_allocation(sizeof(uint32_t), (void**) &a)) return -1;

    // test assignment and read
    *a = 5;
    if(*a != 5) return -1;

    if(test_free(sizeof(uint32_t), a)) return -1;

    // try to free again, make sure this FAILS
    if( !kfree(a) ) return -1;


    //allocate some larger arrays
    uint32_t *b, *c, *d;
    size_t NB = 1000;
    size_t NC = 16000;
    size_t ND = 1000;
        
    if(test_allocation(sizeof(uint32_t)*NB, (void**)&b)) return -1;
    if(test_allocation(sizeof(uint32_t)*NC, (void**)&c)) return -1;
    if(test_allocation(sizeof(uint32_t)*ND, (void**)&d)) return -1;

    //test read and write
    for(size_t nn=0; nn<NB; nn++)
        b[nn] = (uint32_t) nn;
    for(size_t nn=0; nn<ND; nn++)
        d[nn] = (uint32_t) nn;
    for(size_t nn=0; nn<NB; nn++)
        if(b[nn] != (uint32_t) nn) return -1;
    for(size_t nn=0; nn<ND; nn++)
        if(d[nn] != (uint32_t) nn) return -1;


    // test aligned allocation
    uint32_t* e = kmalloc_aligned(sizeof(uint32_t), 4096); // align to page
    if(!e) return -1;
    if( (size_t) e % 4096 ) return -1;
    if((*e = 128) != 128) return -1;

    if(test_free(sizeof(uint32_t)*NB, b)) return -1;
    if(test_free(sizeof(uint32_t)*NC, c)) return -1;
    if(test_free(sizeof(uint32_t)*ND, d)) return -1;
    if(test_free(sizeof(uint32_t), e)) return -1;

    return 0;
}