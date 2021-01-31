#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#define __aligned __attribute__ ((aligned (32)))

void panic(char* str);


size_t strlen(const char* str);
void *memcpy(void *dest, const void *src, size_t n);
void* memset(void* addr, int val, size_t cnt);

int sprintf(char* str, const char* format, ...);


int atoi(const char* str);
char* itoa( int value, char* str, int base);


static inline void bp()
{
  __asm__ ("xchg %bx, %bx");
}

static inline size_t align_int(size_t len, size_t alignment)
{
    size_t am = alignment - 1;
    return (((size_t)len + am) & ~am);
}

static inline void* align_ptr(void* addr, size_t alignment)
{
    // alignment must be increment of 4
    if(alignment % sizeof(void*) != 0 ) return NULL;
    return (void*) align_int((size_t) addr, alignment);
}

static inline int is_pow_of_two(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}


#endif