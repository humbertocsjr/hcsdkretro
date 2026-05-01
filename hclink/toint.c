#include "link.h"

int toint(char *str)
{
    int value = 0;
    bool negative = false;
    while (*str == ' ' || *str == '\t' || *str == '-')
    {
        if (*str == '-')
            negative = !negative;
        str++;
    }
    if (str[0] == '0' && str[1] == 'b')
    {
        str += 2;
        while (*str >= '0' && *str <= '1')
        {
            value <<= 1;
            value += *str++ - '0';
        }
    }
    else if (str[0] == '0' && str[1] == 'o')
    {
        str += 2;
        while (*str >= '0' && *str <= '7')
        {
            value <<= 3;
            value += *str++ - '0';
        }
    }
    else if (str[0] == '0' && str[1] == 'x')
    {
        str += 2;
        while ((*str >= '0' && *str <= '9') || (*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F'))
        {
            value <<= 4;
            if (*str >= 'A' && *str <= 'F')
                value += *str++ - 'A' + 10;
            else if (*str >= 'a' && *str <= 'f')
                value += *str++ - 'a' + 10;
            else
                value += *str++ - '0';
        }
    }
    else if (str[0] == '0')
    {
        while (*str >= '0' && *str <= '7')
        {
            value <<= 3;
            value += *str++ - '0';
        }
    }
    else
    {
        while (*str >= '0' && *str <= '9')
        {
            value *= 10;
            value += *str++ - '0';
        }
    }
    if (negative)
        value = -value;
    return value;
}