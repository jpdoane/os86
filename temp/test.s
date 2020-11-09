.intel_syntax noprefix
.section .text

# void test_loop(int32_t* x, int32_t n)
.globl test_loop
.align 4
.type test_loop, @function
test_loop:
    # Save stack state
    push ebp
    mov ebp, esp 

    #callee saved reg
    push edi

    mov edi, [ebp+8]    # array pointer
    mov eax, [ebp+12]   # array length

loop:
    mov [edi], eax
    inc edi
    dec eax
    jnz loop


    #restore callee saved reg
    pop edi            

    # restore stack state
    mov esp, ebp 
    pop ebp     
    ret
