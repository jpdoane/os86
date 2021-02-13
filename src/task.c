#include "task.h"
#include "kmalloc.h"
#include "common.h"
#include "gdt.h"
#include "mman.h"

task_control_block_t boot_task;
task_control_block_t* current_task;

uint32_t pid_counter;


int initialize_multitasking()
{   
    boot_task.pid = pid_counter = 0;
    boot_task.state = RUNNING;

    //this is initialized only on task switch
    boot_task.esp = NULL;
    boot_task.esp0 = NULL;

    boot_task.cr3 = get_physaddr(&boot_task.pd);

    // this is the only task so far
    //as we add tasks this becomes circular linked list
    boot_task.next_task = &boot_task;
    current_task = &boot_task;

    return 0;
}

task_control_block_t* new_kernel_task(int32_t (*task_entry) (void))
{
    task_control_block_t* new_task = new_tcb();

    uint32_t* esp = (uint32_t*) (new_task->kstacks + 1); //set esp at the end of the stack (stack is empty)
    new_task->cr3 = get_physaddr(&new_task->pd);

    // init new stack with valid context to launch new task
    *(--esp) = (uint32_t) new_task; //argument to launch_task
    *(--esp) = (uint32_t) terminate_task; // launch_task() returns to here, to close out task
    *(--esp) = (uint32_t) launch_task; //common entry function for new tasks
    *(--esp) = 0; //push ebx
    *(--esp) = 0; //push esi
    *(--esp) = 0; //push edi
    *(--esp) = 0; //push ebp

    // update_kstack(new_stack); //update tss with esp0

    //create copy of kernel page_dir
    memcpy((void*) &new_task->pd, (void*) &paging->dir,PAGE_SIZE);

    new_task->pid = ++pid_counter;
    new_task->state = WAITING;
    new_task->esp = esp;
    new_task->esp0 = NULL; //we are already in ring0, so no syscalls...?
    new_task->task_entry = task_entry;
    new_task->return_val = -1;

    //insert task into the ll
    new_task->next_task = current_task->next_task;
    current_task->next_task = new_task;

    return new_task;
}

void yield(void)
{
    current_task->state = WAITING;
    current_task->next_task->state = RUNNING;    
    switch_to_task(current_task->next_task);
}

int32_t join(task_control_block_t* task, int* ret_val)
{
    // make sure we are not trying to join the current task or the main kernel task
    if(task == current_task || task == &boot_task)
        return -1; 

    while(task->state == WAITING || task->state == RUNNING)
        yield();

    if(ret_val)
        *ret_val = task->return_val;

    free_tcb(task);
    return 0;
}


void launch_task(task_control_block_t* task)
{
    task->state = RUNNING;

   // call task main function
    task->return_val = (*task->task_entry)();

    // back from task

    //remove this task from active task list
    task_control_block_t* t = task->next_task;
    while( t->next_task != task)
        t = t->next_task;
    t->next_task = t->next_task->next_task;

    task->state = COMPLETE;

    // this will "return" to terminite_task 
}

int num_tasks()
{
    task_control_block_t* t = current_task;
    int nn = 1;
    while( t->next_task != current_task)
    {
        t = t->next_task;
        nn++;
    }
    return nn;
}
