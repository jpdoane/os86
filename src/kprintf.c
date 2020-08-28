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

int format_int(int value, char* str, unsigned int base, unsigned int min_width, int precision, struct format_flags flags);

int kprint(const char* str, size_t size)
{
    terminal_write(&stdout, str, size);
    return size;
}

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
            kprint("%",1);
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

            //parse type
            char str_buf[512];
            char* val_str = str_buf;
            int len_val_str = 0;
            unsigned int base = 10;
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
                            len_val_str = format_int(va_arg(valist, int), val_str, 10, field_width, field_prec, flags);
                            if(len_val_str<0) return len_val_str; //conversion error
                            break;
                        case(LONGLONGINT):
                            return -1; //long long not supported
                        default:
                            return -1; //invalid length mod
                    }

                    kprint(val_str,len_val_str);
                    nchar += len_val_str;
                    break;

                case 'o':
                case 'u':
                case 'x':
                case 'X':
                case 'p':
                    //signed int
                    switch(length_mod)
                    {
                        case(CHARINT):
                        case(SHORTINT):
                        case(LONGINT):
                        case(DEFAULT):
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
                            len_val_str = format_int(va_arg(valist, unsigned int), val_str, base, field_width, field_prec, flags);
                            if(len_val_str<0) return len_val_str; //conversion error
                            break;
                        case(LONGLONGINT):
                            return -1; //long long not supported
                        default:
                            return -1; //invalid length mod
                    }

                    kprint(val_str,len_val_str);
                    nchar += len_val_str;
                    break;

                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case 'g':
                case 'G':
                    return -1; // not yet implemented
                case 'c':
                    val_str[0] = (char) va_arg(valist, int);

                    if(field_width>1 && !flags.left)    
                    {
                        nchar += field_width - 1;
                        for(int nn=0; nn<field_width-1; nn++)
                            kprint(" ",1);
                    }

                    kprint(val_str,1);
                    nchar++;

                    if(field_width>1 && flags.left)    
                    {
                        nchar += field_width - 1;
                        for(int nn=0; nn<field_width-1; nn++)
                            kprint(" ",1);
                    }
                    
                    break;

                case 's':
                    val_str = va_arg(valist, char*);
                    len_val_str = strlen(val_str);
                    if(field_prec > 0 && field_prec < len_val_str)
                        len_val_str = field_prec;

                    if(field_width>len_val_str && !flags.left)    
                    {
                        nchar += field_width-len_val_str;
                        for(int nn=0; nn<field_width-len_val_str; nn++)
                            kprint(" ",1);
                    }

                    kprint(val_str,len_val_str);
                    nchar += len_val_str;

                    if(field_width>len_val_str && flags.left)
                    {
                        nchar += field_width-len_val_str;
                        for(int nn=0; nn<field_width-len_val_str; nn++)
                            kprint(" ",1);
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
            kprint(f++, 1);
            nchar++;
        }
    }

   /* clean memory reserved for valist */
   va_end(valist);

   return nchar; //number of characters copied to str
}


//returns length of formatted string
int format_int(int value, char* str, unsigned int base, unsigned int min_width, int precision, struct format_flags flags)
{
    if(base < 2 || base > 32)
        return -1;

    //When 0 is printed with an explicit precision 0, the output is empty.
    if(value==0 && precision==0)
    {
        str[0] = 0;
        return 0;
    }

    unsigned int residual; //holds the unsigned value still to be parsed
    char sign_char = 0; //sign char may be +,-, space, or null

    if(base == 10)
    {
        if(value < 0)
        {
            sign_char = '-';
            residual = -value;
        }
        else
        {
            residual = value;

            if(flags.sign)
                sign_char = '+';  //+ overrides space
            else if(flags.space)
                sign_char = ' ';
        }
        

    }
    else
        residual = (unsigned int) value; //all non-decimal numbers are treated as unsigned


    // find length of field
    int char_digits = numdigits_uint(residual, base);
    int char_prec_digits = char_digits>precision ? char_digits : precision;
    int char_sign = sign_char==0 ? 0 : 1;
    int char_radix = 0;

    //add room for radix fmt
    if(flags.alt)
    {
        if(base == 16)
            char_radix = 2; //leading "0x" for hex
        if(base == 8 && char_prec_digits==char_digits && residual>0)
            char_prec_digits++; //force leading 0 for octal
    }

    //total width;
    unsigned int width = char_prec_digits+char_sign+char_radix;
    unsigned int char_padding = 0;
    if(width < min_width)
    {
        //If a precision is given with a numeric conversion the 0 flag is ignored. 
        if(flags.zero && !flags.left && precision<0)
            char_prec_digits += min_width-width;    //pad with zeros
        else
            char_padding = min_width-width;         //pad with spaces
        width = min_width;
    }

    //start writing out string
    char* s = str;
    str[width] = '\0'; //null termination

    //write padding
    if(flags.left)
    {
        for(unsigned int nn=width-char_padding; nn<width; nn++)
            str[nn] = ' ';        
    }
    else
    {
        for(unsigned int nn=0; nn<char_padding; nn++)
            str[nn] = ' ';
        s += char_padding;
    }

    //at this point, s points at the beginning of the field, after any left-side padding

    //write hex prefix or sign character
    if(char_radix>0)
    {
        *(s++) = '0';
        *(s++) = 'x';
    }
    if(char_sign > 0)
    {
        *(s++) = sign_char;
    }

    //at this point, s points at most sig. digit
    //move to least sig digit

    s += char_prec_digits-1;

    //compute each digit from lowest to highest.
    unsigned int digit;
    for(int nn=0;nn<char_prec_digits; nn++)
    {
        digit = residual % base;
        residual /= base;

        //write digit into string and decrement pointer
        if(digit < 10)
            *(s--) = digit + '0';
        else
            *(s--) = digit - 10 + 'A';        
    }

    return width;
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
    kprintf("Oct:     [%-#15o]\n", num);
    kprintf("String:  [%-15s]\n", "Hello World!");
    kprintf("Char:    [%-15c]\n", '!');

    kprintf("\nFixed Precision:\n");
    kprintf("Decimal: [%15.10d]\nHex:     [%#15.10x]\nOct:     [%#15.10o]\nString:  [%15.10s]\nChar:    [%15.10c]\n", 
        num, num, num, "Hello World!",'!');

    kprintf("\nLeading Zeros:\n");
    kprintf("Decimal: [%015.10d]\nHex:     [%#015.10x]\nOct:     [%#015.10o]\n",num, num, num);

    kprintf("\nSigned (space):\n");
    kprintf("Decimal: [% d]\nHex:     [%# x]\nOct:     [%# o]\n",num, num, num);
    kprintf("\nSigned (+):\n");
    kprintf("Decimal: [%+d]\nHex:     [%#+x]\nOct:     [%#+o]\n",num, num, num);
    kprintf("\nSigned: (-)\n");
    kprintf("Decimal: [%+d]\nHex:     [%#+x]\nOct:     [%#+o]\n",-num, -num, -num);

}


