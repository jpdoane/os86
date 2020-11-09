.intel_syntax noprefix
.section .text

# void enable_paging(page_directory_t* page_dir)
.globl enable_paging
.align 4
.type enable_paging, @function
enable_paging:
    # Save stack state
    push ebp
    mov ebp, esp 

    # move arg 1 (page dir) to cr3
    mov eax, [ebp+8]
    mov cr3, eax            

    # set PG and PE bits of cr0
    mov eax, cr0            
    or  eax, 0x80000001
    mov cr0, eax

    # restore stack state
    mov esp, ebp 
    pop ebp     
    ret


# void set_page_dir(page_directory_t* page_dir)
.globl set_page_dir
.align 4
.type set_page_dir, @function
set_page_dir:
    # move arg 1 (page dir) to cr3
    mov eax, [esp+8]
    mov cr3, eax            
    ret

# page_directory_t* get_page_dir()
.globl get_page_dir
.align 4
.type get_page_dir, @function
get_page_dir:
    mov eax, cr3
    ret
