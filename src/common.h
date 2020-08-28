#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>


size_t strlen(const char* str);
void *memcpy(void *dest, const void *src, size_t n);

int sprintf(char* str, const char* format, ...);

int atoi(const char* str);
char* itoa( int value, char* str, int base);

#endif