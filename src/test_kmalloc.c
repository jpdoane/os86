#include "test.h"
#include "kmalloc.h"
#include "kprintf.h"

int test_kmalloc()
{
    uint32_t* a = kmalloc(sizeof(uint32_t));
    if(!a) return -1;
    *a = 5;
    if(*a != 5) return -1;

    int NB = 10000;
    uint32_t* b = kmalloc(sizeof(uint32_t)*NB);
    if(!b) return -1;
    for(int nn=0; nn<NB; nn++)
        b[nn] = (uint32_t) nn;
    for(int nn=0; nn<NB; nn++)
        if(b[nn] != (uint32_t) nn) return -1;

    uint32_t* c = kmalloc_aligned(sizeof(uint32_t), 4096); // align to page
    if(!c) return -1;
    if( (size_t) c % 4096 ) return -1;

    if( kfree(a) ) return -1;
    if( kfree(b) ) return -1;

    int ND = 100;
    uint32_t* d;
    for(int nn=0; nn<100; nn++)
    {
        d = kmalloc(sizeof(uint32_t)*ND);
        if(!d) return -1;
        d[0] = 1;
        d[ND-1] = 1;
        if( kfree(d) ) return -1;
    }

    if( kfree(c) ) return -1;

    return 0;
}