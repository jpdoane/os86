#include "task.h"
#include "paging.h"
#include "mman.h"
#include "common.h"
#include "gdt.h"

task_control_block_t* current_task;
task_control_block_t initial_task;

uint32_t pid_counter;

void initialize_multitasking()
{    
    // initialize current task as initial task
    initial_task.pid = pid_counter = 0;
    initial_task.state = RUNNING;
    initial_task.esp = NULL; //this is initialized only on task switch
    initial_task.pd = get_page_dir();

    // this is the only task so far
    //as we add tasks this becomes circular linked list
    initial_task.next_task = &initial_task;
    current_task = &initial_task;
}

task_control_block_t* new_kernel_task(void (*task_entry) (void))
{
    task_control_block_t* new_task = kmalloc(sizeof(task_control_block_t));
    void* new_stack = kmalloc(KERNEL_STACK_SIZE);
    uint32_t* esp0 = (uint32_t*) (new_stack + KERNEL_STACK_SIZE);

    // init new stack with valid context and new eip
    *(--esp0) = (uint32_t) task_entry; //push eip
    *(--esp0) = 0; //push ebx
    *(--esp0) = 0; //push esi
    *(--esp0) = 0; //push edi
    *(--esp0) = 0; //push ebp

    update_kstack(esp0); //update tss with esp0

    //create copy of kernel page_dir
    void* new_pd = kmalloc_aligned(PAGE_SIZE, PAGE_SIZE);
    memcpy(new_pd, (void*) &pd->dir,PAGE_SIZE);
    new_task->pd = get_physaddr(new_pd); //page dir is physical address.


    new_task->pid = ++pid_counter;
    new_task->state = WAITING;
    new_task->esp = esp0;

    //insert task into the ll
    new_task->next_task = current_task->next_task;
    current_task->next_task = new_task;

    return new_task;
}

void switch_to_next_task(void)
{
    switch_to_task(current_task->next_task);
}
