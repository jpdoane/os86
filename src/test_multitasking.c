#include "test.h"
#include "kprintf.h"
#include "task.h"

#define MUTLITASKING_LOOPS 32

int shared_value;  // tasks share the same kernel memory

int32_t mytask()
{
	int nn;
	for(nn=0;nn<MUTLITASKING_LOOPS;nn++)
	{
		// check that shared_value has been updated by the other task
		if(shared_value != nn)
			return  -1;

		shared_value--;	//decrement, check in other task
		yield();
	}
	return nn;
}

int test_multitasking()
{
	task_control_block_t* task2 = new_kernel_task( &mytask );

	int nn;
	for(nn=0;nn<MUTLITASKING_LOOPS;nn++)
	{
		//set value that will be checked in other task
		shared_value = nn;

		yield();

		// confirm that other task decremented this...
		if(shared_value != nn-1)
			return  -1;
	}

	int ret_val;
	if(join(task2, &ret_val))
		return -1;

    if(ret_val != MUTLITASKING_LOOPS) return -1;

    return 0;
}

