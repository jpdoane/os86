.intel_syntax noprefix

#include "interrupt.h"

.section .text

.global load_idt
.align 4
.extern idtd
.type load_idt, @function
load_idt:
  # mov eax, [esp+4]
	lidt [idtd]
	sti
ret

//get PF addr
.align 4
.global pf_addr
.type pf_addr, @function
pf_addr:
  mov eax, cr2
ret


//exception handlers
// save registers and call exception handler with expection number and (optional) code as arguments

.macro EXCEPTION_HANDLER_NOCODE num
.align 4
.global except_\num
.type except_\num, @function
except_\num:
  push ebp
  mov ebp,esp

// stack:
// ebp+0:   old ebp     <-- current esp, ebp
// ebp+4:   return eip (fault location) <-- esp on entry

  pushad    //save all regs onto stack

  // param 3: fault location
  mov eax,[ebp+4]
  push eax 

  // param 2: (dummy) code number
  mov eax,0
  push eax 

  // param 1: exception number
  mov eax, \num
  push eax 

  call exception_handler
  add esp, 12 //remove 3 params from stack

  popad     //restore all registers
  pop ebp   //restore ebp

  iret
.endm

.macro EXCEPTION_HANDLER_CODE num
.align 4
.global except_\num
.type except_\num, @function
except_\num:
  push ebp
  mov ebp,esp

// stack:
// ebp+0:   old ebp         <-- current esp, ebp
// ebp+4:   exception code  <-- esp on entry
// ebp+8:   return eip (fault location)

  pushad    //save all regs onto stack

  // param 3: fault location
  mov eax,[ebp+8]
  push eax 

  // param 2: code number
  mov eax,[ebp+4]
  push eax 

  // param 1: exception number
  mov eax, \num
  push eax 

  call exception_handler
  add esp, 12 //remove 3 params from stack

  popad     //restore all registers
  pop ebp   //restore ebp

  add esp, 4 //pop code off stack
  iret
.endm

EXCEPTION_HANDLER_NOCODE 0 // DBZ
EXCEPTION_HANDLER_NOCODE 1 // debug
EXCEPTION_HANDLER_NOCODE 2 // NMI
EXCEPTION_HANDLER_NOCODE 3 // breakpoint
EXCEPTION_HANDLER_NOCODE 4 // overflow
EXCEPTION_HANDLER_NOCODE 5 // bound range
EXCEPTION_HANDLER_NOCODE 6 // invalid opcode
EXCEPTION_HANDLER_NOCODE 7 // device not available
EXCEPTION_HANDLER_CODE 8 //   double fault
EXCEPTION_HANDLER_CODE 10 //  invalid tss
EXCEPTION_HANDLER_CODE 11 //  segment not present
EXCEPTION_HANDLER_CODE 12 //  stack segment fault
EXCEPTION_HANDLER_CODE 13 //  gen protection fault
EXCEPTION_HANDLER_CODE 14 //  page fault
EXCEPTION_HANDLER_CODE 16 //  x87 fp exception
EXCEPTION_HANDLER_CODE 17 //  alignemt check
EXCEPTION_HANDLER_CODE 18 //  machine check
EXCEPTION_HANDLER_CODE 19 //  simd fp ex
EXCEPTION_HANDLER_CODE 20 //  virt ex
EXCEPTION_HANDLER_CODE 30 //  security ex


// define irq handlers
.macro IRQ_HANDLER num
.align 4
.global irq_\num
.type irq_\num, @function
irq_\num:
  pushad          //save all regs
  mov eax, \num
  push eax        // irq number
  call irq_handler
  add esp, 4 //clean up stack
  popad           //restore all registers
  iret
.endm

IRQ_HANDLER 0 
IRQ_HANDLER 1
IRQ_HANDLER 2
IRQ_HANDLER 3
IRQ_HANDLER 4
IRQ_HANDLER 5
IRQ_HANDLER 6
IRQ_HANDLER 7
IRQ_HANDLER 8
IRQ_HANDLER 9
IRQ_HANDLER 10
IRQ_HANDLER 11
IRQ_HANDLER 12
IRQ_HANDLER 13
IRQ_HANDLER 14
IRQ_HANDLER 15
