
#include "test.h"
#include "kprintf.h"

int print_testresult(int status, char* message)
{
    uint8_t color = terminal_getcolor(&stdout);
    kprintf("[");
    if(status)
    {
        terminal_setcolor(&stdout, VGA_COLOR_RED);
        kprintf("FAILED");
    }
    else
    {
        terminal_setcolor(&stdout, VGA_COLOR_GREEN);
        kprintf("SUCCESS");
    }

    terminal_setcolor(&stdout, color);
    kprintf("] ");

    kprintf(message);
    kprintf("\n");
    
    return status;
}

// returns zero on all pass, or negative of number failed tests
int run_unit_tests()
{
    kprintf("\nRunning Unit Tests...\n");
    int passed = 0;
    int total = 0;
    passed = print_testresult(test_kmalloc(), "kmalloc") ? 0 : passed+1;
    total++;
    passed = print_testresult(test_multitasking(), "Multitasking") ? 0 : passed+1;
    total++;

    kprintf("%d of %d Unit tests passed", passed, total);

    if(passed != total)
        return passed-total;

    return 0;
}