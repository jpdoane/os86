#include "kmalloc.h"
#include "mman.h"
#include "common.h"

heap_block_t* kheap_head = NULL;

int kmalloc_init()
{
    kheap_head = ksbrk(HEAP_INCREMENT);
   
    if(!kheap_head)
        return -1;

    kheap_head = align_ptr(kheap_head,HEAP_BLOCKSIZE_UNITS);

    kheap_head->magic = HEAP_BLOCK_MAGIC;
    kheap_head->status = HEAP_BLOCK_FREE;
    kheap_head->prev = kheap_head;
    kheap_head->next = kheap_head;

    kheap_head->size = ((heap_block_t*) ksbrk(0)) - kheap_head;

    if(kheap_head->size < HEAP_INCREMENT/HEAP_BLOCKSIZE_UNITS)
        return -1;    

    return 0;    
}

// insert new_block into LL between b and b->next
// return 0 on success 
int kheap_insert_block(heap_block_t* b, heap_block_t* new_block)
{
    
    if(kheap_validate_block(b) || kheap_validate_block(new_block))
    {
        panic("kernel heap corrupted");
        return -1;
    }

    new_block->prev = b;
    new_block->next = b->next;            
    b->next->prev = new_block;
    b->next = new_block;

    return 0;
}


// remove block b from LL
// return pointer to previous block, NULL on error
heap_block_t* kheap_delete_block(heap_block_t* b)
{
    if(kheap_validate_block(b->prev) || kheap_validate_block(b->next))
    {
        panic("kernel heap corrupted");
        return NULL;
    }

    b->prev->next = b->next;
    b->next->prev = b->prev;

    return b->prev;
}

// free block and merge with free neighbors
// return ptr to new free block
heap_block_t* kheap_free_block(heap_block_t* b)
{
    //check magic numbers to make sure theres no overrun
    if(kheap_validate_block(b))
    {
        panic("kernel heap corrupted");
        return NULL;
    }

    // make sure this block isnt already free allocated
    if(b->status == HEAP_BLOCK_FREE)
        return NULL;

    b->status = HEAP_BLOCK_FREE;

    //merge with next block if free
    if(b->next == HEAP_BLOCK_FREE && b->next != kheap_head)
    {
        b->size += b->next->size;
        if(!kheap_delete_block(b->next)) return NULL;
    }

    //merge with previous block if free, and set b = b->prev
    if(b->prev == HEAP_BLOCK_FREE && b != kheap_head)
    {
        b->prev->size += b->size;
        b = kheap_delete_block(b);
    }

    return b;
}


// maintain b->size >= size*HEAP_BLOCKSIZE_UNITS bytes
// if there is enough extra room, split b into two
// return new size of block on successful split (in HEAP_BLOCKSIZE_UNITS)
// return 0 on insufficinet size for splitting into new block (b->size < size+1)
// return <0 on error
int kheap_split_block(heap_block_t* b, size_t size)
{
    //check magic numbers to make sure theres no overrun
    if(kheap_validate_block(b))
    {
        panic("kernel heap corrupted");
        return -1;
    }

    if(b->size < size + 1 ) return 0;

    // create new empty block
    heap_block_t* new_block = b + size;
    new_block->magic = HEAP_BLOCK_MAGIC;
    new_block->status = HEAP_BLOCK_FREE;
    new_block->size = b->size - size;

    // insert new block to into LL after b
    if(kheap_insert_block(b,new_block)) return -1;

    // shrink b and return new size
    return b->size = size;
}


// grow the kernel heap by at least minimum_block_size*HEAP_BLOCKSIZE_UNITS bytes
// and return new tail block that is guaranteeed to be at least minimum_block_size
// return NULL on error or OOM
heap_block_t* grow_kheap(size_t minimum_block_size)
{
    size_t grow_size = minimum_block_size*HEAP_BLOCKSIZE_UNITS > HEAP_INCREMENT ? minimum_block_size*HEAP_BLOCKSIZE_UNITS : HEAP_INCREMENT;
    heap_block_t* new_block = ksbrk(grow_size);
    if(!new_block)
        return NULL; // error in ksbrk, e.g. out of memory

    //ksbrk doesnt guarantee properly aligned blocks, so make sure we are aligned
    new_block = align_ptr(new_block,HEAP_BLOCKSIZE_UNITS);
    
    new_block->magic = HEAP_BLOCK_MAGIC;
    new_block->status = HEAP_BLOCK_FREE;

    // get end of block from ksbrk(0), and again make sure it is aligned.
    // this time align with a mask so that aligned address is <= ksbrk(0)
    new_block->size = new_block - (heap_block_t*) ( (uint32_t) ksbrk(0) & HEAP_BLOCKSIZE_MASK);

    //doublecheck that new block is large enough
    if(new_block->size < minimum_block_size) return NULL;

    kheap_insert_block(kheap_head->prev, new_block); //add new block to tail
    return new_block;
}

void* kmalloc(size_t sz)
{
    //convert to HEAP_BLOCKSIZE_UNITS, rounding up, and add one for header
    size_t block_size = 1 + (sz+ HEAP_BLOCKSIZE_UNITS -1)/HEAP_BLOCKSIZE_UNITS;
    
    if(!kheap_head)
    {
        //initialize the heap with some space
        if(kmalloc_init())
            panic("Error initializing kernel heap");
    }

    heap_block_t* block = kheap_head;

    while(block->status != HEAP_BLOCK_FREE || block->size < block_size)
    {
        //if we've returned to the head, we have run through the entire list without finding a free block
        if(block->next == kheap_head)
        {
            //we've reached the end of the heap, so add a new block at the end
            block = grow_kheap(block_size);
            if(!block) return NULL; //grow_kheap() failed, e.g. OOM 
        }
        else
        {
            block = block->next;
        }
    }

    // block is a free block that is large enough for this allocation

    //split block if its big enough
    if(kheap_split_block(block, block_size)<0)
        panic("Critical error in malloc()");

    block->status = HEAP_BLOCK_USED;
    return block+1; // return pointer after header
}

void* kmalloc_aligned(size_t sz, size_t alignment)
{
    //alignement must be multiple of HEAP_BLOCKSIZE_UNITS
    if(alignment % HEAP_BLOCKSIZE_UNITS) return NULL;

    //convert alignment to HEAP_BLOCKSIZE_UNITS
    size_t block_alignment = alignment/HEAP_BLOCKSIZE_UNITS;

    //convert sz to HEAP_BLOCKSIZE_UNITS, rounding up, and add one for header
    size_t block_size = 1 + (sz+ HEAP_BLOCKSIZE_UNITS -1)/HEAP_BLOCKSIZE_UNITS;

    
    if(!kheap_head)
    {
        //initialize the heap with some space
        if(!kmalloc_init())
            panic("Error initializing kernel heap");
    }

    heap_block_t* block = kheap_head;

    //how many additional units are needed to align this block?
    size_t alignment_space;

    while(true)
    {
        if(block->status == HEAP_BLOCK_FREE)
        {
            // where would aligned data start in this block?
            size_t block_data = (size_t)block/HEAP_BLOCKSIZE_UNITS + 1;  //(unaligned) data starts after header
            size_t am = block_alignment - 1;
            size_t block_data_aligned = ((block_data + am) & ~am); //align data

            alignment_space = block_data_aligned-block_data; 

            if(block->size >= block_size + alignment_space)
                break;

        }

        //if we've returned to the head, we have run through the entire list without finding a free block
        if(block->next == kheap_head)
        {
            //we've reached the end of the heap, so add a new block at the end
            block = grow_kheap(block_size);
            if(!block) return NULL; //grow_kheap() failed, e.g. OOM 
        }
        else
        {
            block = block->next;
        }
    }

    // block is a free block that is large enough for this allocation

    //if we need to realign, split off the first part of the block
    if(alignment_space>0)
    {
        if(kheap_split_block(block, alignment_space)<0)
            panic("Critical error in malloc()");
        block = block->next;
    }
    
    //split end of block off its big enough
    if(kheap_split_block(block, block_size)<0)
        panic("Critical error in malloc()");

    block->status = HEAP_BLOCK_USED;
    return block+1; // return pointer after header
}



// free memory previously allocated with kmalloc
// return 0 on success.
// return -1 on fail (e.g. if p was not allocated with malloc, or memory corruption detected)
int kfree(void* p)
{
    //get pointer to block header
    heap_block_t* b = ((heap_block_t*) p) - 1;

    if(kheap_validate_block(b))
    {
        panic("kernel heap corrupted");
        return -1;
    }

    if(kheap_free_block(b)) return 0;

    return -1;
}


// size of allocated and free mem in heap
// returns 0 on success
int kheap_size(size_t* allocated, size_t* free)
{
    *allocated = 0;
    *free = 0;

    // make sure heap is initilialized
    if(!kheap_head)
        return -1;

    heap_block_t* block = kheap_head;
    while(true)
    {        
        if(kheap_validate_block(block))
            return -1;        

        if(block->status == HEAP_BLOCK_FREE)
            *free += block->size*HEAP_BLOCKSIZE_UNITS;
        else if(block->status == HEAP_BLOCK_USED)
            *allocated += block->size*HEAP_BLOCKSIZE_UNITS;
        else
            return -1;        

        block = block->next;
        if(block == kheap_head)
            return 0;
    }

}
