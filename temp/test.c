#include <stdio.h>
#include <stdint.h>

void test_loop(int32_t* x, int32_t n);

int main(void)
{
    int32_t a[10];
    test_loop(a,10);

    for(int nn=0;nn<10;nn++)
        printf("a[%d] = %d", nn, a[nn]);

   return 0; 
}