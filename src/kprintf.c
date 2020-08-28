#include "kprintf.h"

struct format_flags
{
    unsigned char alt;
    unsigned char zero;
    unsigned char left;
    unsigned char space;
    unsigned char sign;
};

//returns number of digits of unsigned int represented in given base
//num_digits is always >= 1
unsigned int numdigits_uint(unsigned int value, unsigned int base);

int format_uint(unsigned int value, char* str, unsigned int base, int precision, int cap);

int kprint(const char* str)
{
    terminal_writestring(&stdout, str);
    return strlen(str);
}

int kprintn(const char* str, size_t len)
{
    terminal_write(&stdout, str, len);
    return len;
}

int kprint_char(char c)
{
    terminal_putchar(&stdout, c); 
    return 1;
}

int kprintn_char(char c, size_t rpt)
{
    terminal_putcharn(&stdout, c, rpt); 
    return rpt;
}





//     if(base == 10)
//     {
//         if(value < 0)
//         {
//             v = -value;
//             sign_char = '-';
//         }
//         else
//         {
//             v = value;
//             if(sign < 0)
//                 sign_char = ' ';
//             else if(sign>0)
//                 sign_char = '+';
//             //if sign==0 then positive number has no sign character
//         }
//     }
//     else
//         v = (unsigned int) value; //all non-decimal numbers are treated as unsigned


//     // find number of digits
//     int digits = numdigits_uint(v, base);
//     int num_digits = digits>precision ? digits : precision;
//     int num_digits_signed = sign_char == '\0' ? num_digits : num_digits + 1;

//     //write formatted number out in reverse order
//     char* s = str + num_digits_signed;

//     *(s--) = '\0'; //null termination

//     //compute each digit from lowest to highest.
//     unsigned int d;
//     for(int nn=0;nn<num_digits; nn++)
//     {
//         d = v % base;
//         v /= base;

//         //write digit into string and decrement pointer
//         if(d < 10)
//             *(s--) = d + '0';
//         else if(radix_cap == 0)
//             *(s--) = d - 10 + 'a';
//         else
//             *(s--) = d - 10 + 'A';
//     }

//     if(sign_char != '\0')
//         *s = sign_char;

//     return num_digits_signed;
// }





int kprintf(const char* format, ...)
{
    /* initialize valist for num number of arguments */
    va_list valist;
    va_start(valist, format);

    // parse format into str
    const char* f = format;
    int nchar = 0;
    while(f[0] != 0)
    {
        if(f[0] == '\\' && f[1] == '%')
        {
            //write escaped "%%" as '%'
            kprint_char('%');
            nchar++;
            f += 2;
        }

        if(f[0] == '%')
        {
            // found new field

            // parse flags
            struct format_flags flags = {0};
            int check_flags = 1;
            while(check_flags)
            {
                switch(*(++f))
                {
                    case '#':
                        if(flags.alt) return 0;
                        flags.alt = 1;
                        break;
                    case '0':
                        if(flags.zero) return 0;
                        flags.zero = 1;
                        break;
                    case '-':
                        if(flags.left) return 0;
                        flags.left = 1;
                        break;
                    case ' ':
                        if(flags.space) return 0;
                        flags.space = 1;
                        break;
                    case '+':
                        if(flags.sign) return 0;
                        flags.sign = 1;
                        break;
                    case '\0':
                        //unexpected string termination
                        return -1;
                        break;
                    default:
                        //no more flags
                        check_flags = 0;
                }
            }

            if(flags.left) flags.zero = 0; //If the 0 and - flags both appear, the 0 flag is ignored

            // parse field width
            int field_width=0;
            int nn=0;
            while(f[nn] >= '0' && f[nn] <= '9')
                nn++;            
            if(nn>0)
            {
                field_width = atoi(f);
                f += nn;
            }

            // parse field precision
            int field_prec=-1; //default is -1
            if(f[0] == '.')
            {
                f++;
                nn=0;
                while(f[nn] >= '0' && f[nn] <= '9')
                    nn++;            
                if(nn>0)
                {
                    field_prec = atoi(f);
                    if(field_prec<0) field_prec = 0;
                    f += nn;
                }
            }

            //parse type
            enum length_mod_t {DEFAULT, CHARINT, SHORTINT, LONGINT, LONGLONGINT,
                            LONGDOUBLE, INTMAX, SIZE, PTRDIFF} length_mod;

            switch(*(f++))
            {
                case 'h':
                    if(*f == 'h')
                    {
                        f++;
                        length_mod = CHARINT;
                    }
                    else
                        length_mod = SHORTINT;
                    break;
                case 'l':
                    if(*f == 'l')
                    {
                        f++;
                        length_mod = LONGLONGINT;
                    }
                    else
                        length_mod = LONGINT;
                    break;
                case 'q':
                case 'L':
                    length_mod = LONGDOUBLE;
                    break;
                case 'j':
                    length_mod = INTMAX;
                    break;
                case 'Z':
                case 'z':
                    length_mod = SIZE;
                    break;
                case 't':
                    length_mod = PTRDIFF;
                    break;
                default:
                    f--; //no length mod, back up a char...
                    length_mod = DEFAULT;
            }

            //parse field
            char val_buf[512]; //assuming here that numberical values cannot exceed 512 chars 
            if(field_width>511)
                return -1;

            switch(*f)
            {
                case 'i':
                case 'd':
                    //signed int
                    switch(length_mod)
                    {
                        case(CHARINT):
                        case(SHORTINT):
                        case(LONGINT):
                        case(DEFAULT):
                            {

                                int i_value = va_arg(valist, int);
                                char sign_char = 0; //sign character or 0 if none
                                unsigned int ui_value; //unsigned value (abs value)
                                if(i_value<0)
                                {
                                    ui_value = -i_value;
                                    sign_char = '-';
                                }
                                else
                                {
                                    ui_value = i_value;
                                    if(flags.sign)
                                        sign_char = '+';
                                    else if(flags.space)
                                        sign_char = ' ';
                                    
                                    //else positive number has no sign character
                                }

                                //convert value into string
                                int len = format_uint(ui_value, val_buf, 10, field_prec, 0);
                                //amount of padding required to meet requested width
                                int lenpad = field_width - len; 

                                if(sign_char)
                                {
                                    lenpad--; //remove one char of padding to account for sign
                                    if(flags.zero)
                                        nchar += kprint_char(sign_char); //if zero padding, sign is at beginning
                                }

                                if(!flags.left && lenpad>0)
                                {
                                    if(flags.zero)
                                        nchar += kprintn_char('0', lenpad);
                                    else
                                        nchar += kprintn_char(' ', lenpad);
                                }

                                if(!flags.zero && sign_char)
                                    nchar += kprint_char(sign_char); //if not zero padding, write sign here

                                nchar += kprint(val_buf);

                                if(flags.left && lenpad>0)
                                    nchar += kprintn_char(' ', lenpad);

                            }
                            break;
                        case(LONGLONGINT):
                            return -1; //long long not supported
                        default:
                            return -1; //invalid length mod
                    }
                    break;

                case 'o':
                case 'u':
                case 'x':
                case 'X':
                case 'p':
                    //unsigned int
                    switch(length_mod)
                    {
                        case(CHARINT):
                        case(SHORTINT):
                        case(LONGINT):
                        case(DEFAULT):
                            {
                                unsigned int base = 10;
                                int ui_value = va_arg(valist, unsigned int);

                                if(*f == 'o')
                                    base = 8;
                                else if(*f == 'x' || *f == 'X')
                                    base = 16;
                                else if(*f == 'p')
                                {
                                    //print as hex with 0x prefix
                                    base = 16;
                                    flags.alt = 1; 
                                }
                                else
                                    flags.alt = 0;
                                
                                int cap = (*f == 'X') ? 1 : 0;

                                //convert value into string
                                int len = format_uint(ui_value, val_buf, base, field_prec, cap);
                                //amount of padding required to meet requested width
                                int lenpad = field_width - len; 


                                //format radix prefix for oct and hex with alt flag
                                char radix[3]="";
                                if(flags.alt)
                                {
                                    if(base==16)
                                    {
                                        lenpad -= 2;
                                        radix[0] = '0';
                                        radix[1] = cap ? 'X' : 'x';
                                        radix[2] = 0;
                                    }
                                    else if(base==8 && val_buf[0] != '0')
                                    {
                                        lenpad -= 1;
                                        radix[0] = '0';
                                        radix[1] = 0;
                                    }

                                    //if zero padding, radix is printed first
                                    if(flags.zero)
                                        nchar += kprint(radix);
                                }


                                //print leading padding, if any
                                if(!flags.left && lenpad>0)
                                {
                                    if(flags.zero)
                                        nchar += kprintn_char('0', lenpad);
                                    else
                                        nchar += kprintn_char(' ', lenpad);
                                }

                                //print radix prefix if valid and not leading zeros
                                if(!flags.zero && radix[0])
                                    nchar += kprint(radix);

                                //print number itself
                                nchar += kprint(val_buf);

                                //print trailing padding if left justified
                                if(flags.left && lenpad>0)
                                    nchar += kprintn_char(' ', lenpad);

                            }
                            break;
                        case(LONGLONGINT):
                            return -1; //long long not supported
                        default:
                            return -1; //invalid length mod
                    }

                    break;

                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case 'g':
                case 'G':
                    return -1; // not yet implemented
                case 'c':
                    {
                        char c = (char) va_arg(valist, int);

                        if(field_width>1 && !flags.left)    
                            nchar += kprintn_char(' ', field_width-1);

                        nchar += kprint_char(c);

                        if(field_width>1 && flags.left)    
                            nchar += kprintn_char(' ', field_width-1);
                    }                    
                    break;

                case 's':
                    {
                        char* str = va_arg(valist, char*);
                        int len = strlen(str);
                        if(field_prec > 0 && field_prec < len)
                            len = field_prec;

                        int lenpad = field_width - len; 

                        if(lenpad>0 && !flags.left)    
                            nchar += kprintn_char(' ', lenpad);

                        nchar += kprintn(str,len);

                        if(lenpad>0 && flags.left)
                            nchar += kprintn_char(' ', lenpad);
                        
                    }
                    break;
                default:
                    //unsupported type
                    return -1;
                
            }
            f++;
        }
        else
        {
            //this isnt part of a format code, just copy char to string and increment pointers
            kprint_char(*(f++));
            nchar++;
        }
    }

   /* clean memory reserved for valist */
   va_end(valist);

   return nchar; //number of characters copied to str
}

// formats an unsigned integer as a (alpha)numeric string
// base may be 2-32
// precision is per printf format, precision<0 represents default (unspecified) 
// cap = 0, alpha-numbers are capitalized [base>10 only]
// returns length of string (not counting null termination)
int format_uint(unsigned int value, char* str, unsigned int base, int precision, int cap)
{
    if(base < 2 || base > 32)
        return -1;

    //When 0 is printed with an explicit precision 0, the output is empty.
    if(value==0 && precision==0)
    {
        str[0] = 0;
        return 0;
    }

    // find number of digits
    int num_digits = numdigits_uint(value, base);
    num_digits = num_digits>precision ? num_digits : precision;

    //write formatted number out in reverse order
    char* s = str + num_digits;

    *(s--) = '\0'; //null termination

    //compute each digit from lowest to highest.
    unsigned int d;
    for(int nn=0;nn<num_digits; nn++)
    {
        d = value % base;
        value /= base;

        //write digit into string and decrement pointer
        if(d < 10)
            *(s--) = d + '0';
        else if(cap == 0)
            *(s--) = d - 10 + 'a';
        else
            *(s--) = d - 10 + 'A';
    }

    return num_digits;
}



//returns number of digits of unsigned int represented in given base
//num_digits is always >= 1
unsigned int numdigits_uint(unsigned int value, unsigned int base)
{
    unsigned int num_digits = 1;
    while(value >= base)
    {
        num_digits++;
        value /= base;
    }
    return num_digits;
}



void kprintf_test()
{
    int num = 12345678;    
    kprintf("\nkprintf() tests...\n", num);
    kprintf("Fixed width, right justify:\n");
    kprintf("Decimal: [%15d]\n", num);
    kprintf("Hex:     [%#15x]\n", num);
    kprintf("Oct:     [%#15o]\n", num);
    kprintf("String:  [%15s]\n", "Hello World!");
    kprintf("Char:    [%15c]\n", '!');

    kprintf("\nFixed width, left justify:\n");
    kprintf("Decimal: [%-15d]\n", num);
    kprintf("Hex:     [%-#15x]\n", num);
    kprintf("String:  [%-15s]\n", "Hello World!");

    kprintf("\nFixed Precision:\n");
    kprintf("Decimal: [%15.10d]\nHex:     [%#15.10x]\nOct:     [%#15.10o]\nString:  [%15.10s]\n", 
        num, num, num, "Hello World!");

    kprintf("\nLeading Zeros:\n");
    kprintf("Decimal: [%015.10d]\nHex:     [%#015.10x]\nOct:     [%#015.10o]\n",num, num, num);

    kprintf("Signed (none):  [%d], [%d]\n",num, -num);
    kprintf("Signed (space): [% d], [% d]\n",num, -num);
    kprintf("Signed (+):     [%+d], [%+d]\n",num, -num);

}


