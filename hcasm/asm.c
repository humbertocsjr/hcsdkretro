#include "asm.h"

source_t *_source = NULL;

// [English] Parse a numeric value from a string using the same formats supported by the scanner
// [Portuguese] Parseia um valor numérico de uma string usando os mesmos formatos do scanner
// [English] Supported formats: 0b... (binary), 0o... (octal), 0x... (hex), ...h (hex suffix), decimal
// [Portuguese] Formatos suportados: 0b... (binário), 0o... (octal), 0x... (hex), ...h (sufixo hex), decimal
int parse_numeric_value(const char *str)
{
    if (!str || !*str)
        error("empty numeric value");
    
    int value = 0;
    const char *p = str;
    
    // [English] Binary: 0b... or 0B...
    // [Portuguese] Binário: 0b... ou 0B...
    if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B'))
    {
        p += 2;
        while (*p)
        {
            if (*p != '0' && *p != '1')
                error("invalid binary digit in numeric value: %s", str);
            value = (value << 1) | (*p - '0');
            p++;
        }
        return value;
    }
    
    // [English] Octal: 0o... or 0O...
    // [Portuguese] Octal: 0o... ou 0O...
    if (p[0] == '0' && (p[1] == 'o' || p[1] == 'O'))
    {
        p += 2;
        while (*p)
        {
            if (*p < '0' || *p > '7')
                error("invalid octal digit in numeric value: %s", str);
            value = (value << 3) | (*p - '0');
            p++;
        }
        return value;
    }
    
    // [English] Hexadecimal: 0x... or 0X...
    // [Portuguese] Hexadecimal: 0x... ou 0X...
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    {
        p += 2;
        while (*p)
        {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (*p >= 'a' && *p <= 'f')
                digit = *p - 'a' + 10;
            else if (*p >= 'A' && *p <= 'F')
                digit = *p - 'A' + 10;
            else
                error("invalid hex digit in numeric value: %s", str);
            value = (value << 4) | digit;
            p++;
        }
        return value;
    }
    
    // [English] Check for hex suffix (h or H)
    // [Portuguese] Verifica sufixo hex (h ou H)
    int len = strlen(str);
    if (len > 1 && (str[len - 1] == 'h' || str[len - 1] == 'H'))
    {
        // [English] Validate all chars before 'h' are hex digits
        // [Portuguese] Valida se todos os chars antes de 'h' são dígitos hex
        value = 0;
        for (int i = 0; i < len - 1; i++)
        {
            char c = str[i];
            int digit = -1;
            if (c >= '0' && c <= '9')
                digit = c - '0';
            else if (c >= 'a' && c <= 'f')
                digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                digit = c - 'A' + 10;
            else if (c == '0' && i == 0)
                continue; // Allow leading 0
            else
                error("invalid hex digit in numeric value: %s", str);
            value = (value << 4) | digit;
        }
        return value;
    }
    
    // [English] If starts with 0 but no prefix, treat as octal (if valid) or hex with H suffix
    // [Portuguese] Se começa com 0 mas sem prefixo, trata como octal (se válido) ou hex com H
    if (p[0] == '0' && p[1])
    {
        bool all_octal = true;
        const char *q = p + 1;
        while (*q)
        {
            if (*q < '0' || *q > '7')
            {
                all_octal = false;
                break;
            }
            q++;
        }
        if (all_octal && (p[1] >= '0' && p[1] <= '7'))
        {
            // Octal value
            p++;
            value = 0;
            while (*p)
            {
                value = (value << 3) | (*p - '0');
                p++;
            }
            return value;
        }
    }
    
    // [English] Decimal: standard numeric string
    // [Portuguese] Decimal: string numérica padrão
    value = 0;
    while (*p)
    {
        if (*p < '0' || *p > '9')
            error("invalid decimal digit in numeric value: %s", str);
        value = value * 10 + (*p - '0');
        p++;
    }
    return value;
}

// [English] Show help message with usage info and exit
// [Portuguese] Mostra mensagem de ajuda com informações de uso e sai
void help()
{
    printf("HC Assembler for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hcasm-CPU [ARGS] [SOURCE FILE]\n");
    printf("Arguments:\n");
    printf("-o [FILE]       : Output file (default: a.obj)\n");
    printf("-D NAME=VALUE   : Define a constant (numeric value in assembler format)\n");
    printf("-dump [FILE]    : Dump description of object file (default: NONE)\n");
    exit(1);
}

// [English] Entry point: parse args, open output, parse source, close output
// [Portuguese] Ponto de entrada: analisa argumentos, abre saída, analisa fonte, fecha saída
int main(int argc, char **argv)
{
    char *out_name = NULL;
    char *dump_name = NULL;
    char *in_name = NULL;
    // [English] Parse command-line arguments
    // [Portuguese] Analisa argumentos da linha de comando
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            help();
        }
        else if (!strcmp(argv[i], "-o"))
        {
            i++;
            if (out_name)
                error("output name already defined");
            if (i < argc && argv[i][0] != '-')
                out_name = argv[i];
            else
                error("output name not defined");
        }
        else if (!strcmp(argv[i], "-D"))
        {
            i++;
            if (i >= argc)
                error("-D requires NAME=VALUE argument");
            char *def = argv[i];
            char *eq = strchr(def, '=');
            if (!eq)
                error("-D argument must be in format NAME=VALUE");
            
            // [English] Extract name and value
            // [Portuguese] Extrai nome e valor
            int name_len = eq - def;
            if (name_len <= 0)
                error("-D: name cannot be empty");
            char name[256];
            strncpy(name, def, name_len);
            name[name_len] = 0;
            
            // [English] Validate name is valid identifier (alphanumeric, underscore, dot)
            // [Portuguese] Valida se o nome é um identificador válido
            for (int j = 0; name[j]; j++)
            {
                char c = name[j];
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '.'))
                    error("-D: invalid identifier '%s'", name);
                if (j == 0 && (c >= '0' && c <= '9'))
                    error("-D: identifier cannot start with digit: '%s'", name);
            }
            
            // [English] Parse numeric value
            // [Portuguese] Parseia valor numérico
            int value = parse_numeric_value(eq + 1);
            
            // [English] Register constant
            // [Portuguese] Registra constante
            consts_set(name, value);
        }
        else if (!strcmp(argv[i], "-dump"))
        {
            i++;
            if (dump_name)
                error("dump name already defined");
            if (i < argc)
                dump_name = argv[i];
        }
        else
        {
            if (in_name)
                error("source file name already defined");
            in_name = argv[i];
        }
    }
    if (!out_name)
        out_name = "a.obj";
    if (!in_name)
        help();
    // [English] Open output, parse source, finalize
    // [Portuguese] Abre saída, analisa fonte, finaliza
    out_open(out_name, dump_name);
    parse(in_name);
    out_close();
    return 0;
}
