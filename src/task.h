#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "paging.h"

#define KSTACKS_PER_PROCESS 1
#define KSTACK_PAGES 2

typedef enum {RUNNING, READY, WAITING, COMPLETE, ERROR} process_state_t;

typedef struct kstack_t {
    page_t guard;               // unmapped page, stack overflows result in page fault
    page_t pg[KSTACK_PAGES];    // pages allocated for the stack
} kstack_t;

typedef struct task_control_block_t
{
    uint32_t pid;               // process ID
    process_state_t state;      // 
    void* esp;                  // task stack ptr
    void* esp0;                 // syscall stack ptr
    void* cr3;                  //physical address of pd
    struct task_control_block_t* next_task;       // offset 0x14
    int32_t (*task_entry)(void);
    int32_t return_val;
    page_directory_t pd;
    kstack_t kstacks[KSTACKS_PER_PROCESS];
} task_control_block_t;


int initialize_multitasking();
task_control_block_t* new_kernel_task( int32_t (*task_entry) (void) );
void switch_to_next_task(void);
void switch_to_task(task_control_block_t* task);
void yield();
int32_t join(task_control_block_t* task);


void launch_task(task_control_block_t* task);

int num_tasks();

extern void terminate_task();

#endif