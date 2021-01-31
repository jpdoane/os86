#include "common.h"
#include "kprintf.h"

void panic(char* str)
{
    kprintf(str);
    while(true);
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

// copy n bytes from src to dest
void *memcpy(void *dest, const void *src, size_t n)
{
    //system wordsize is equal to length of size_t
    size_t n_words = n/sizeof(size_t);
    size_t* dest_word = (size_t *) dest;
    size_t* src_word = (size_t *) src;
    for(size_t nn=0; nn<n_words; nn++)
        dest_word[nn]=src_word[nn];
    
    //copy remaining bytes
    size_t n_bytes = n - n_words*sizeof(size_t);
    if(n_bytes > 0)
    {
        char* dest_byte = (char *) dest;
        char* src_byte = (char *) src;
        for(size_t nn=0; nn<n_bytes; nn++)
            dest_byte[nn]=src_byte[nn];
    }

    return dest;
}

void* memset(void* addr, int val, size_t cnt)
{
    char* m = (char*) addr;
    for(size_t nn=0;nn<cnt; nn++)
        m[nn] = val;

    return addr;
}


int atoi(const char* str)
{
    int p = 0;
    int neg = 0;
    //find start of number string, ignore whitespace
    while(str[p] == '-' || str[p] == ' ' || (str[p] >= 9 && str[p] <= 13))
    {
        if(str[p++] == '-')
        {
            neg=1;
            break;
        }
        if(str[p++] == '\0')
            return 0; //invalid string (only whitespace found)
    }

    int num_start = p;

    //find end of integer string (ints only, no decimal point)
    while(str[p] >= '0' && str[p] <= '9')
    {
        if(str[p++] == '\0')
            return 0; //invalid string (only whitespace found)
    }

    if(num_start == p) return 0; //length 0 string

    int number = 0;
    int place = 1;
    //iterate backwards through number to add each digit
    for(int digit = p-1; digit >= num_start; digit--)
    {
        number += (int) (str[digit] - '0') * place;
        place *= 10;
    }

    if(neg)
        return -1*number;
    else
        return number;
}

char* itoa( int value, char* str, int base)
{
    if(base < 2 || base > 32)
        return NULL;

    //handle zero as special case 
    if(value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    char* s = str;
    unsigned int residual; //holds the unsigned value still to be parsed
    if(base == 10 && value < 0)
    {
        residual = -value;
        *(s++) = '-';
    }
    else
    {
        //all non-decimal numbers are treated as unsigned
        residual = (unsigned int) value;
    }
    
    // find number of digits so that we can parse number
    // directly into string from lowest to highest digit    
    unsigned int num_digits = 0;
    unsigned int temp = residual;
    while(temp>0)
    {
        num_digits++;
        temp /= base;
    }

    //point at end of string
    s += num_digits;
    *(s--) = '\0'; //null termination

    //compute each digit from lowest to highest.
    unsigned int digit;
    while(residual>0)
    {
        digit = residual % base;
        residual /= base;

        //write digit into string and decrement pointer
        if(digit < 10)
            *(s--) = digit + '0';
        else
            *(s--) = digit - 10 + 'A';        
    }

    return str;
}
