#include "test.h"
#include "kprintf.h"
#include "task.h"

int32_t test_multitasking2()
{
	int nn;
	for(nn=0;nn<5;nn++)
		yield();
	return nn;
}

int test_multitasking()
{
	task_control_block_t* task2 = new_kernel_task( &test_multitasking2 );

	int nn;
	for(nn=0;nn<10;nn++)
		yield();

	join(task2);

    if(task2->return_val == 5) return 0;

    return -1;
}
