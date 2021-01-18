#include "regs.h"
#include "kprintf.h"

void printregs(const reg_t* regs)
{
    kprintf("Registers:\n");
    kprintf("eax: 0x%x\n", regs->eax);
    kprintf("ebx: 0x%x\n", regs->ebx);
    kprintf("ecx: 0x%x\n", regs->ecx);
    kprintf("edx: 0x%x\n", regs->edx);
    kprintf("esi: 0x%x\n", regs->esi);
    kprintf("edi: 0x%x\n", regs->edi);
    kprintf("ebp: 0x%x\n", regs->ebp);
    kprintf("esp: 0x%x\n", regs->esp);
    kprintf("eip: 0x%x\n", regs->eip);

    kprintf("eflags: CF=%d PF=%d AF=%d ZF=%d SF=%d TF=%d\n", (regs->eflags&EFLAGS_CF) == EFLAGS_CF,
                                                            (regs->eflags&EFLAGS_PF) == EFLAGS_PF,
                                                            (regs->eflags&EFLAGS_AF) == EFLAGS_AF,
                                                            (regs->eflags&EFLAGS_ZF) == EFLAGS_ZF,
                                                            (regs->eflags&EFLAGS_SF) == EFLAGS_SF,
                                                            (regs->eflags&EFLAGS_TF) == EFLAGS_TF);
    kprintf("        IF=%d DF=%d OF=%d IOPL=%d NT=%d RF=%d\n", (regs->eflags&EFLAGS_IF) == EFLAGS_IF,
                                                            (regs->eflags&EFLAGS_DF) == EFLAGS_DF,
                                                            (regs->eflags&EFLAGS_OF) == EFLAGS_OF,
                                                            (regs->eflags&EFLAGS_IOPL) >> 12,
                                                            (regs->eflags&EFLAGS_NT) == EFLAGS_NT,
                                                            (regs->eflags&EFLAGS_RF) == EFLAGS_RF);
    kprintf("        VM=%d AC=%d VIF=%d VIP=%d ID=%d\n", (regs->eflags&EFLAGS_VM) == EFLAGS_VM,
                                                            (regs->eflags&EFLAGS_AC) == EFLAGS_AC,
                                                            (regs->eflags&EFLAGS_VIF) == EFLAGS_VIF,
                                                            (regs->eflags&EFLAGS_VIP) == EFLAGS_VIP,
                                                            (regs->eflags&EFLAGS_ID) == EFLAGS_ID);
    kprintf("cr0: 0x%x\n", regs->cr0);
    kprintf("cr2: 0x%x\n", regs->cr2);
    kprintf("cr3: 0x%x\n", regs->cr3);
    kprintf("cr4: 0x%x\n", regs->cr4);
}
