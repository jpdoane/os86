#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>
#include <stdint.h>
#include "mman.h"

#define HEAP_BLOCK_MAGIC 0x0123beef  //only 28bits, so 1st nibble must = 0
#define HEAP_BLOCK_FREE 0
#define HEAP_BLOCK_USED 1

#define HEAP_INCREMENT PAGE_SIZE*4    //increment heap size in increments of 4 pages


typedef struct heap_block_t {
    //pack magic number (28bits) + status (4 bits) into single 32bit word
    uint32_t magic : 28; 
    uint32_t status : 4;

    struct heap_block_t* prev;
    struct heap_block_t* next;
    size_t size;    //block size (including header) in units of sizeof(heap_block_t)
} heap_block_t;

// block header size is 16 bytes = sizeof(heap_block_t)
// blocks must be aligned to this size, and internally to kmalloc, block size is measured in these units
// this makes pointer math simple, e.g. block->next = block[block->size]
#define HEAP_BLOCKSIZE_UNITS sizeof(heap_block_t)
#define HEAP_BLOCKSIZE_MASK ~(HEAP_BLOCKSIZE_UNITS-1)

int kmalloc_init();
void* kmalloc(size_t sz);
void* kmalloc_aligned(size_t sz, size_t alignment);
int kfree(void* p);

// size of allocated and free mem in heap
// returns 0 on success
int kheap_size(size_t* allocated, size_t* free);


// check magic number
static inline int kheap_validate_block(heap_block_t* b)
{
    return b->magic != HEAP_BLOCK_MAGIC;
}


#endif