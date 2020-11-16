#ifndef __GDT_H__
#define __GDT_H__

#define SEGMENT_LIMIT_LOW_MASK       0x0FFFF
#define SEGMENT_LIMIT_HIGH_MASK      0xF0000
#define SEGMENT_BASE_LOW_MASK        0x0000FFFF
#define SEGMENT_BASE_MID_MASK        0x00FF0000
#define SEGMENT_BASE_HIGH_MASK       0xFF000000

#define SEGMENT_ACCESS_PRESENT       1 << 7
#define SEGMENT_ACCESS_PRIV          3 << 5
#define SEGMENT_ACCESS_TYPE          1 << 4
#define SEGMENT_ACCESS_EXECUTABLE    1 << 3
#define SEGMENT_ACCESS_DC            1 << 2
#define SEGMENT_ACCESS_READWRITE     1 << 1
#define SEGMENT_ACCESS_ACCESSED      1

#define SEGMENT_FLAGS_GRANULARITY    1 << 3
#define SEGMENT_FLAGS_SIZE           1 << 2

#ifndef ASM_FILE
#include <stdint.h>

typedef struct __attribute__((__packed__)) gdt_description_t
{
    uint16_t size;
    uint32_t addr;
} gdt_description_t;


// jeez this thing is a mess. Given that there are packed 4-bit
// fields, lets just define the whole thing as a bitfield
typedef struct __attribute__((__packed__)) segment_descriptor_t
{
    uint32_t limit_low  : 16;   // limit[0:15]
    uint32_t base_low   : 16;   // base[0:15]
    uint32_t base_mid   : 8;    // base[23:16]
    uint32_t access     : 8;
    uint32_t limit_high : 4;    // limit[16:19]
    uint32_t flags      : 4;
    uint32_t base_high  : 8;    // base[24:31]
} segment_descriptor_t;

inline void populate_segment_descriptor(segment_descriptor_t* sd, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    sd->limit_low = limit & SEGMENT_LIMIT_LOW_MASK;
    sd->limit_high = (limit & SEGMENT_LIMIT_HIGH_MASK) >> 16;
    sd->base_low = base & SEGMENT_BASE_LOW_MASK;
    sd->base_mid = (base & SEGMENT_BASE_MID_MASK) >> 16;
    sd->base_high = (base & SEGMENT_BASE_HIGH_MASK) >> 24;
    sd->access = access;
    sd->flags = flags;
}

// defined in gdt_asm.S
void load_gdt(gdt_description_t* gdt_desc);

#endif
#endif