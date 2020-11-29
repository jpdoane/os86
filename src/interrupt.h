#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#ifndef ASM_FILE
#include <stdint.h>
#include "io.h"

#define NUM_INTERRUPT_HANDLERS 256

#define PIC1    0x20
#define PIC2    0xA0
#define PIC1_CMD    PIC1
#define PIC1_DATA   (PIC1+1)
#define PIC2_CMD    PIC2
#define PIC2_DATA   (PIC2+1)

//first send init cmd
#define PIC_INIT    0x11
//then send offset
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28
//then send master/slave id/addr
#define PIC1_MS 0x4
#define PIC2_MS 0x2
//then send 8086 mode
#define PIC_8086MODE 0x01
//then send IRQ masks (1 is mask, 0 is allow)


#define PIC_EOI 0x20 //send after interrupt is handled

typedef struct __attribute__((__packed__))  idt_entry_t
{
    uint16_t offset_low;
    uint16_t segment;
    uint8_t zero;
    uint8_t type;
    uint16_t offset_high;
} idt_entry_t;

typedef struct __attribute__((__packed__))  idt_descriptor_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__ ((aligned (32)))  idt_descriptor_t;

extern idt_entry_t idt[];

void init_interrupts();
// void load_idt(idt_descriptor_t* idt);
void load_idt();

void set_interrupt_handler(int int_num, void* handler);
void exception_handler(uint32_t exception_num, uint32_t code);
void irq_handler(uint32_t irq_num);


extern void except_0();
extern void except_1();
extern void except_2();
extern void except_3();
extern void except_4();
extern void except_5();
extern void except_6();
extern void except_7();
extern void except_8();
extern void except_10();
extern void except_11();
extern void except_12();
extern void except_13();
extern void except_14();
extern void except_16();
extern void except_17();
extern void except_18();
extern void except_19();
extern void except_20();
extern void except_30();

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

#endif
#endif