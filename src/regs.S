.intel_syntax noprefix

.section .text

.global getregs
.align 4
.type getregs, @function
getregs:
    push ebp
    mov ebp,esp

    // stack:
    // ebp+0:   old ebp     <-- current esp, ebp
    // ebp+4:   return eip  <-- esp on entry
    // ebp+8:   param 1: reg_t*

    push ebx //save ebx
    mov ebx, [ebp+8] //reg_t*

    mov [ebx+0], eax //save eax
    mov eax, [esp] //get ebx from stack (dont worry about preserving eax, saved by caller)
    mov [ebx+4], eax //save ebx
    mov [ebx+8], ecx
    mov [ebx+12], edx
    mov [ebx+16], esi
    mov [ebx+20], edi
    mov eax, ebp        
    add eax, 12         // esp of caller is ebp + 12
    mov [ebx+24], eax   //save caller esp
    mov eax, [ebp]      // rather confusingly, ther original ebp is pointed at by current ebp
    mov [ebx+28], eax   //save ebp
    mov eax, [ebp+4]    //eip of return addr
    mov [ebx+32], eax   //save caller eip
    mov eax, 0
    lahf
    mov [ebx+36], eax //save low flags
    mov eax, cr0    
    mov [ebx+40], eax //save cr0
    mov eax, cr2    
    mov [ebx+44], eax //save cr2
    mov eax, cr3    
    mov [ebx+48], eax //save cr3
    mov eax, cr4    
    mov [ebx+52], eax //save cr4

    pop ebx
    pop ebp

	ret
