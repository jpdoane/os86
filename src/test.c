
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
