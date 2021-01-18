#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#define EFLAGS_CF 1
#define EFLAGS_PF 1<<2
#define EFLAGS_AF 1<<4
#define EFLAGS_ZF 1<<6
#define EFLAGS_SF 1<<7
#define EFLAGS_TF 1<<8
#define EFLAGS_IF 1<<9
#define EFLAGS_DF 1<<10
#define EFLAGS_OF 1<<11
#define EFLAGS_IOPL 3<<12
#define EFLAGS_NT 1<<14
#define EFLAGS_RF 1<<16
#define EFLAGS_VM 1<<17
#define EFLAGS_AC 1<<18
#define EFLAGS_VIF 1<<19
#define EFLAGS_VIP 1<<20
#define EFLAGS_ID 1<<21

typedef struct reg_t
{
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;  
  uint32_t esi;
  uint32_t edi;
  uint32_t esp;
  uint32_t ebp;
  uint32_t eip;
  uint32_t eflags;
  uint32_t cr0;
  // uint32_t cr1;
  uint32_t cr2;
  uint32_t cr3;
  uint32_t cr4;
} reg_t;


void getregs(reg_t* regs);
void printregs(const reg_t* regs);


#endif