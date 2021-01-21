#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>

void* kmalloc(size_t sz);
void* kmalloc_aligned(size_t sz, size_t alignment);

#endif