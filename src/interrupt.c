#include "interrupt.h"
#include "common.h"
#include "kprintf.h"
#include "keyboard.h"
#include "gdt.h"

idt_entry_t __aligned idt[NUM_INTERRUPT_HANDLERS] = {0};
idt_descriptor_t __aligned idtd;

void init_interrupts()
{
    /* init the PIC */
	outb(PIC_INIT, PIC1_CMD);  // init PIC 1
    outb(PIC_INIT, PIC2_CMD);  // init PIC 2
    outb(PIC1_OFFSET, PIC1_DATA); // map irq 0x0-0x7 to int 0x20-0x27
    outb(PIC2_OFFSET, PIC2_DATA); // map irq 0x8-0xf to int 0x28-0x2f
    outb(PIC1_MS, PIC1_DATA);     // pic1 is master, with slave at irq 2
    outb(PIC2_MS, PIC2_DATA);     // pic2 is slave with id 2
    outb(PIC_8086MODE, PIC1_DATA); //enable 8086 mode
    outb(PIC_8086MODE, PIC2_DATA); //enable 8086 mode
    // interrupt mask
    outb( ~(1<<1) , PIC1_DATA); //enable irq1 (keyboard)
    outb(0xff, PIC2_DATA); //disable interrupts from slave


    // individual handler routines defined in interrupt.S
    // these call back to exception_handler() and irq_handler()
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
    load_idt();
}

void set_interrupt_handler(int int_num, void* handler)
{
	idt[int_num].offset_low = ((uint32_t) handler) & 0xffff;
	idt[int_num].segment = KERNEL_CODE_SEGMENT;
	idt[int_num].zero = 0;
	idt[int_num].type = 0x8e; /* INTERRUPT_GATE */
	idt[int_num].offset_high = (((uint32_t) handler) & 0xffff0000) >> 16;    
}

void page_fault_handler(uint32_t code, void* fault_ip)
{

    if(code & PF_CODE_WRITE)
        kprintf("\n\nPage Fault writing to 0x%x (instruction 0x%x)\n", pf_addr(), fault_ip);
    else        
        kprintf("\n\nPage Fault reading from 0x%x (instruction 0x%x)\n", pf_addr(), fault_ip);
    if(code & PF_CODE_PGPROT)
        kprintf("Page protection violation\n");
    else
        kprintf("Page not present\n");
    if(code & PF_CODE_USER)
        kprintf("\n\nFault caused by user code\n");
    if(code & PF_CODE_RES)
        kprintf("Page reserved bit set\n");
    if(code & PF_CODE_IF)
        kprintf("Fault caused by instruction fetch\n");

    while(1);
}


void exception_handler(uint32_t exception_num, uint32_t code, void* fault_ip)
{
    if(exception_num==14)
        page_fault_handler(code, fault_ip);
    else
        kprintf("\n\nException %u with code 0x%x at instruction 0x%x\n", exception_num, code, fault_ip);
    while(1);
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

    // notify PIC that interrupt handling is complete
    if(irq_num >= 8)
        outb(PIC_EOI, PIC2_CMD);

    outb(PIC_EOI, PIC1_CMD);

    return;
}
