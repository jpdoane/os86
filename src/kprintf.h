#ifndef __KPRINTF_H__
#define __KPRINTF_H__

#include "common.h"
#include "terminal.h"

int kprintf(const char* format, ...);
int kprint_char(char c);

#endif
