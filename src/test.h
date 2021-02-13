
#ifndef __TEST_H__
#define __TEST_H__

int test_kmalloc();
int test_multitasking();

int print_testresult(int status, char* message);

// returns zero on all pass, or negative of number failed tests
int run_unit_tests();


#endif