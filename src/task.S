.intel_syntax noprefix

// global task structs
.extern kernel_main_task
.extern current_task
.extern tss

.section .text


// void switch_to_task(task_control_block_t* new_process)
.globl switch_to_task
.align 4
.type switch_to_task, @function
switch_to_task:

    #save old task state
    # ;  For cdecl; EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    # ;  EIP is already saved on the stack by the caller's "CALL" instruction
    # ;  The task isn't able to change CR3 so it doesn't need to be saved
    # ;  Segment registers are constants (while running kernel code) so they don't need to be saved
    push ebx
    push esi
    push edi
    push ebp

    // save current stack ptr in curent TCB
    mov edi, current_task     // load the address of curent task in edi (current_task is a pointer)
    mov [edi+0x8], esp          // tcb.esp = 0x8

    // new task is passed as parameter.  Load as current_task
    mov esi, [esp+(4+1)*4]
    mov current_task, esi

    //load new stack, pd, and kernel stack from new TCB
    mov esp, [esi+0x8]          // stack ptr
    mov ebx, [esi+0xc]          // top of kernel stack
    mov eax, [esi+0x10]         // page dir
    
    // update tss with new esp0
    mov [tss + 0x4], ebx         // tss.esp0 = 0x4

    // update cr3 if the virtual address space has changed
    mov ecx,cr3
    cmp eax,ecx
    je .done
    mov cr3,eax
.done:

    // we now are in new address space with new stack pointer

    // restore task context from stack
    pop ebp
    pop edi
    pop esi
    pop ebx
 
    // task eip is now at top of stack
    // "return" to new task
ret


.global terminate_task
terminate_task:
    // terminate task and return to main kernel thread
    lea esi, [kernel_main_task]
    mov current_task, esi

    //load kernel_main_task stack, pd, and kernel stack from new TCB
    mov esp, [esi+0x8]          // stack ptr
    mov ebx, [esi+0xc]          // top of kernel stack
    mov eax, [esi+0x10]         // page dir
    
    // update tss with new esp0
    mov [tss + 0x4], ebx         // tss.esp0 = 0x4

    // update cr3 if the virtual address space has changed
    mov ecx,cr3
    cmp eax,ecx
    je .done
    mov cr3,eax
.done2:

    // we now are in new address space with new stack pointer
    // restore task context from stack
    pop ebp
    pop edi
    pop esi
    pop ebx
 
    // "return" to kmain
ret
