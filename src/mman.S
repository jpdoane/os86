.intel_syntax noprefix

.section .text


// int relocate_stack(void* old_stack_base, void* new stack_base);
.globl relocate_stack
.align 4
.type relocate_stack, @function
relocate_stack:

    push ebp
    mov ebp, esp 

    // stack:
    // ebp+0:   old ebp     <-- current esp, ebp
    // ebp+4:   return eip  <-- esp on entry
    // ebp+8:   param 1: old_stack_base
    // ebp+12:   param 2: new_stack_base
    
    mov ecx, [ebp+8]    // current stack base
    mov eax, [ebp+12]   // new stack base
    sub eax, ecx        // stack offset

    push eax            // push eax on stack, after we move we will pop it off to make sure this worked...

    add esp, eax        // add offset to stack ptr

    pop ebx             // pop off from new stack.  This should be eax...
    pop ebp             // restore ebp and add offset
    add ebp, eax        // add offset to base ptr

    sub eax, ebx        // eax = 0 if this worked... this is out return value

ret
