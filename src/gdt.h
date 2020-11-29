#ifndef __GDT_H__
#define __GDT_H__

#define KERNEL_CODE_SEGMENT 0x8
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT 0x18
#define USER_DATA_SEGMENT 0x20
#define TSS_SEGMENT 0x28


#define SEGMENT_LIMIT_LOW_MASK       0x0FFFF
#define SEGMENT_LIMIT_HIGH_MASK      0xF0000
#define SEGMENT_BASE_LOW_MASK        0x0000FFFF
#define SEGMENT_BASE_MID_MASK        0x00FF0000
#define SEGMENT_BASE_HIGH_MASK       0xFF000000


// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
// #define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
// #define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)
 
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

#define SEG_FLAG_SIZE(x)      ((x) << 0x02) // Size (0 for 16-bit, 1 for 32)
#define SEG_FLAG_GRAN(x)      ((x) << 0x03) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)

#define SEG_TSS             0x89
#define SEG_FLAG_TSS        0x4

#ifndef ASM_FILE
#include <stdint.h>

typedef struct __attribute__((__packed__)) gdt_description_t
{
    uint16_t size;
    uint32_t addr;
} gdt_description_t;


// jeez this thing is a mess. Given that there are packed 4-bit
// fields, lets just define the whole thing as a bitfield
typedef struct __attribute__((__packed__)) gdt_entry_t
{
    uint32_t limit_low  : 16;   // limit[0:15]
    uint32_t base_low   : 16;   // base[0:15]
    uint32_t base_mid   : 8;    // base[23:16]
    uint32_t access     : 8;
    uint32_t limit_high : 4;    // limit[16:19]
    uint32_t flags      : 4;
    uint32_t base_high  : 8;    // base[24:31]
} gdt_entry_t;


typedef struct tss_entry_t
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now.. 
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         
   uint32_t cs;        
   uint32_t ss;        
   uint32_t ds;        
   uint32_t fs;       
   uint32_t gs;         
   uint32_t ldt;      
   uint16_t trap;
   uint16_t iomap_base;
} tss_entry_t;


inline void populate_gdt_entry(gdt_entry_t* sd, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    sd->limit_low = limit & SEGMENT_LIMIT_LOW_MASK;
    sd->limit_high = (limit & SEGMENT_LIMIT_HIGH_MASK) >> 16;
    sd->base_low = base & SEGMENT_BASE_LOW_MASK;
    sd->base_mid = (base & SEGMENT_BASE_MID_MASK) >> 16;
    sd->base_high = (base & SEGMENT_BASE_HIGH_MASK) >> 24;
    sd->access = access;
    sd->flags = flags;
}

void init_gdt();
void update_tss(uint32_t kernel_stack_ptr);

// defined in gdt_asm.S
void load_gdt();
void load_tss();

#endif
#endif