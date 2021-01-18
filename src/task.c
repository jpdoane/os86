#include "task.h"
#include "paging.h"
#include "mman.h"
#include "common.h"
#include "gdt.h"

task_control_block_t* current_task;
task_control_block_t kernel_main_task;

uint32_t pid_counter;

void initialize_multitasking()
{    
    // initialize current task as initial task
    kernel_main_task.pid = pid_counter = 0;
    kernel_main_task.state = RUNNING;
    kernel_main_task.esp = NULL; //this is initialized only on task switch
    kernel_main_task.pd = get_page_dir();

    // this is the only task so far
    //as we add tasks this becomes circular linked list
    kernel_main_task.next_task = &kernel_main_task;
    current_task = &kernel_main_task;
}

task_control_block_t* new_kernel_task(int32_t (*task_entry) (void))
{
    task_control_block_t* new_task = kmalloc(sizeof(task_control_block_t));
    void* new_stack = kmalloc(KERNEL_STACK_SIZE);
    uint32_t* esp0 = (uint32_t*) (new_stack + KERNEL_STACK_SIZE);

    // init new stack with valid context to launch new task
    *(--esp0) = (uint32_t) new_task; //argument to launch_task
    *(--esp0) = (uint32_t) terminate_task; // launch_task() returns to here, to close out task
    *(--esp0) = (uint32_t) launch_task; //common entry function for new tasks
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

int32_t join(task_control_block_t* task)
{
    // make sure we are not trying to join the current task or the main kernel task
    if(task == current_task || task == &kernel_main_task)
        return -1; 

    while(task->state == WAITING || task->state == RUNNING)
        yield();

    // free task memory: stack, heap and paging structures
    // free tcb? << what should lifetime be?  Want to interrogate after process completes, but dont want it to live forever...
    // before I can do any of this, I need to actually implement free() :)

    return 0;
}


void launch_task(task_control_block_t* task)
{
    task->state = RUNNING;

   // call task main function
    task->return_val = (*task->task_entry)();

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
