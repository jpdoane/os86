#include "interrupt.h"
#include "common.h"
#include "kprintf.h"
#include "keyboard.h"

idt_entry_t idt[NUM_INTERRUPT_HANDLERS] = {0};
idt_descriptor_t idtd __attribute__ ((aligned (32)));

void init_interrupts()
{
    /* init the PIC */
	outb(PIC_INIT, PIC1_CMD);
    outb(PIC_INIT, PIC2_CMD);
    outb(PIC1_OFFSET, PIC1_DATA);
    outb(PIC2_OFFSET, PIC2_DATA);
    outb(PIC1_MS, PIC1_DATA);
    outb(PIC2_MS, PIC2_DATA);
    outb(PIC_8086MODE, PIC1_DATA);
    outb(PIC_8086MODE, PIC2_DATA);
    outb( ~(1<<1) , PIC1_DATA); //enable irq1 (keyboard)
    outb(0xff, PIC2_DATA); //disable interrupts from slave


    set_interrupt_handler(0,except_0);
    set_interrupt_handler(1,except_1);
    set_interrupt_handler(2,except_2);
    set_interrupt_handler(3,except_3);
    set_interrupt_handler(4,except_4);
    set_interrupt_handler(5,except_5);
    set_interrupt_handler(6,except_6);
    set_interrupt_handler(7,except_7);
    set_interrupt_handler(8,except_8);
    set_interrupt_handler(10,except_10);
    set_interrupt_handler(11,except_11);
    set_interrupt_handler(12,except_12);
    set_interrupt_handler(13,except_13);
    set_interrupt_handler(14,except_14);
    set_interrupt_handler(16,except_16);
    set_interrupt_handler(17,except_17);
    set_interrupt_handler(18,except_18);
    set_interrupt_handler(19,except_19);
    set_interrupt_handler(20,except_20);
    set_interrupt_handler(30,except_30);

    set_interrupt_handler(32,irq_0);
    set_interrupt_handler(33,irq_1);
    set_interrupt_handler(34,irq_2);
    set_interrupt_handler(35,irq_3);
    set_interrupt_handler(36,irq_4);
    set_interrupt_handler(37,irq_5);
    set_interrupt_handler(38,irq_6);
    set_interrupt_handler(39,irq_7);
    set_interrupt_handler(40,irq_8);
    set_interrupt_handler(41,irq_9);
    set_interrupt_handler(42,irq_10);
    set_interrupt_handler(43,irq_11);
    set_interrupt_handler(44,irq_12);
    set_interrupt_handler(45,irq_13);
    set_interrupt_handler(46,irq_14);
    set_interrupt_handler(47,irq_15);



    idtd.base = (uint32_t) idt;
    idtd.limit = sizeof(idt_entry_t) * NUM_INTERRUPT_HANDLERS - 1;
    load_idt(&idtd);
}

void set_interrupt_handler(int int_num, void* handler)
{
	idt[int_num].offset_low = ((uint32_t) handler) & 0xffff;
	idt[int_num].segment = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	idt[int_num].zero = 0;
	idt[int_num].type = 0x8e; /* INTERRUPT_GATE */
	idt[int_num].offset_high = (((uint32_t) handler) & 0xffff0000) >> 16;    
}

void exception_handler(uint32_t exception_num, uint32_t code)
{
    kprintf("Exception %u with code %u\n", exception_num, code);
}

void irq_handler(uint32_t irq_num)
{
    if(irq_num == 1)
    {
        unsigned char scan_code = inb(0x60);
        char key_ascii = scancode_to_ascii(scan_code);
        // kprintf("code: 0x%x, ascii:%c\n", scan_code, key_ascii);
        if(key_ascii>0)
            kprint_char(key_ascii);
    
    }
    eoi();
    return;
}
