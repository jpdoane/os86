#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include "paging.h"

typedef enum {RUNNING, READY, WAITING, COMPLETE, ERROR} process_state_t;

typedef struct task_control_block_t
{
    uint32_t pid;               // offset 0x00
    process_state_t state;      // offset 0x04
    void* esp;               // offset 0x08
    void* esp0;               // offset 0x0c
    page_directory_t* pd;       // offset 0x10
    struct task_control_block_t* next_task;       // offset 0x14
    int32_t (*task_entry)(void);
    int32_t return_val;
} task_control_block_t;



void initialize_multitasking();
task_control_block_t* new_kernel_task( int32_t (*task_entry) (void) );
void switch_to_next_task(void);
void switch_to_task(task_control_block_t* task);
void yield();
int32_t join(task_control_block_t* task);


void launch_task(task_control_block_t* task);

int num_tasks();

extern void terminate_task();

#endif