# record memory table from bios
# each entry in table is 24 bytes:
#   u64 base addr
#   u64 region size
#   u32 region type
#   u32 ACPI 3.0 extended attributes
# 
# arguments:
#   1) pointer to table (relative to register es)
# returns number of entries saved in table

.intel_syntax noprefix
#  .code16gcc


.section .text
.globl memTable
.align 4
.type memTable, @function

memTable:
    # Save stack state
    push ebp
    mov ebp, esp 
    # callee-saved registers 
    push edi     
    push esi     
    push ebx     

    mov di, [ebp+8]        # arg 1: 16 bit table address
    #set es=0
    mov ax, 0
    mov es, ax

    # set up first-time 0xe820 bios call
    mov ebx, 0x0            # continuation: 0 for first entry
    mov edx, 0x534D4150     # magic number ('SMAP')
    mov esi,  0             # count number of entries 

    e820_call:
    mov eax, 0xe820         # function code
    mov ecx, 24             # buffer entry size: 24 bytes 
    clc                     # Clear carry flag
    int 0x15                # call bios
    jc short error
    cmp eax, edx            # eax should have same magic number
    jne short error
    inc esi                 # received valid entry, increment count
    cmp ecx, 20             # ecx has actual size of entry (always >= 20)
    jg short has_acpi_attr
    mov dword ptr es:[di + 20], 1 # if acpi attr not returned, then set manually
    has_acpi_attr:
    cmp ebx, 0              # ebx=0 indicates end of table
    je short end_of_table

    # get next entry
    add di, 24              # increment table pointer
    jmp short e820_call

    error:
    mov eax, -1             # return error
    clc
    jmp short return

    end_of_table:
    mov eax, esi             # return number of entries

return:
    # restore callee-saved registers 
    pop ebx
    pop esi     
    pop edi     
    # restore stack state
    mov esp, ebp 
    pop ebp     
    ret
