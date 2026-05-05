#include "link.h"

// [English] Convert a string to an integer, supporting binary (0b), octal (0o, 0), hex (0x), and decimal
// [Portuguese] Converte uma string para inteiro, suportando binário (0b), octal (0o, 0), hex (0x) e decimal
int toint(char *str)
{
    int value = 0;
    bool negative = false;
    // [English] Handle leading whitespace and negation signs
    // [Portuguese] Processa espaços iniciais e sinais de negação
    while (*str == ' ' || *str == '\t' || *str == '-')
    {
        if (*str == '-')
            negative = !negative;
        str++;
    }
    // [English] Parse binary (0b prefix)
    // [Portuguese] Analisa binário (prefixo 0b)
    if (str[0] == '0' && str[1] == 'b')
    {
        str += 2;
        while (*str >= '0' && *str <= '1')
        {
            value <<= 1;
            value += *str++ - '0';
        }
    }
    // [English] Parse octal (0o prefix)
    // [Portuguese] Analisa octal (prefixo 0o)
    else if (str[0] == '0' && str[1] == 'o')
    {
        str += 2;
        while (*str >= '0' && *str <= '7')
        {
            value <<= 3;
            value += *str++ - '0';
        }
    }
    // [English] Parse hexadecimal (0x prefix)
    // [Portuguese] Analisa hexadecimal (prefixo 0x)
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
    // [English] Parse octal (legacy 0 prefix, digits 0-7 only)
    // [Portuguese] Analisa octal (prefixo legado 0, apenas dígitos 0-7)
    else if (str[0] == '0')
    {
        while (*str >= '0' && *str <= '7')
        {
            value <<= 3;
            value += *str++ - '0';
        }
    }
    // [English] Parse decimal (no special prefix)
    // [Portuguese] Analisa decimal (sem prefixo especial)
    else
    {
        while (*str >= '0' && *str <= '9')
        {
            value *= 10;
            value += *str++ - '0';
        }
    }
    // [English] Apply negation if toggled
    // [Portuguese] Aplica negação se alternada
    if (negative)
        value = -value;
    return value;
}
