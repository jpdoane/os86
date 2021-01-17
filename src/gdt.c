#include "gdt.h"
#include "common.h"

gdt_entry_t __aligned gdt[6] = {0};
gdt_description_t __aligned  gdtd;

tss_entry_t tss = {0};

void init_gdt()
{
    // gdt[0] unused
    populate_gdt_entry(&gdt[1], 0, 0xffffffff, SEG_PRES(1) | SEG_PRIV(0) | SEG_DESCTYPE(1) | SEG_CODE_EXRD, SEG_FLAG_GRAN(1) | SEG_FLAG_SIZE(1)); // priv code, seg 0x008
    populate_gdt_entry(&gdt[2], 0, 0xffffffff, SEG_PRES(1) | SEG_PRIV(0) | SEG_DESCTYPE(1) | SEG_DATA_RDWR, SEG_FLAG_GRAN(1) | SEG_FLAG_SIZE(1)); // priv data, seg 0x010
    populate_gdt_entry(&gdt[3], 0, 0xffffffff, SEG_PRES(1) | SEG_PRIV(3) | SEG_DESCTYPE(1) | SEG_CODE_EXRD, SEG_FLAG_GRAN(1) | SEG_FLAG_SIZE(1)); // user code, seg 0x018
    populate_gdt_entry(&gdt[4], 0, 0xffffffff, SEG_PRES(1) | SEG_PRIV(3) | SEG_DESCTYPE(1) | SEG_DATA_RDWR, SEG_FLAG_GRAN(1) | SEG_FLAG_SIZE(1)); // user data  seg 0x020
    populate_gdt_entry(&gdt[5], (uint32_t) &tss, sizeof(tss_entry_t), SEG_TSS, SEG_FLAG_TSS); // tss, seg 0x028

    //tss.esp0 = XXXXX
    tss.ss0 = KERNEL_DATA_SEGMENT; //kernel data segment    
    tss.iomap_base = sizeof(tss_entry_t);
    // before we can actually switch tasks, we must first set the kernel stack pointer, with update_tss()

    gdtd.addr = (uint32_t) gdt;
    gdtd.size = sizeof(gdt);    
    load_gdt(&gdtd);
}

void update_kstack(void* kernel_stack_ptr)
{
    tss.esp0 = (uint32_t) kernel_stack_ptr; //kernel stack ptr
    load_tss();
}
