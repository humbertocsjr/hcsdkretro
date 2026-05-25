#include "bcomp.h"

#define MAX_MACROS 256
#define MACRO_NAME 64
#define MACRO_VAL 4096

// [English] Structure representing a preprocessor macro definition
// [Portuguese] Estrutura que representa uma definição de macro do pré-processador
typedef struct
{
    char name[MACRO_NAME];
    char value[MACRO_VAL];
    int nargs;
    char args[8][MACRO_NAME];
} macro_t;

static macro_t macros[MAX_MACROS];
static int nmacros = 0;
static int cond_stack[64];
static int cond_sp = 0;
static int cond_skip = 0;

static char *incl_dirs[32];
static int nincl = 0;

// [English] Adds a new preprocessor define (macro with no arguments).
// Stores the name and value, marks it as object-like (nargs = -1).
// [Portuguese] Adiciona uma nova definição de pré-processador (macro sem argumentos).
// Armazena o nome e valor, marca como macro do tipo objeto (nargs = -1).
void preproc_add_define(const char *name, const char *value)
{
    if (nmacros >= MAX_MACROS)
        return;
    strncpy(macros[nmacros].name, name, MACRO_NAME - 1);
    strncpy(macros[nmacros].value, value, MACRO_VAL - 1);
    macros[nmacros].nargs = -1;
    nmacros++;
}

// [English] Searches for a macro by name using linear search
// [Portuguese] Busca uma macro pelo nome usando busca linear
static macro_t *find_macro(const char *name)
{
    for (int i = 0; i < nmacros; i++)
        if (!strcmp(macros[i].name, name))
            return &macros[i];
    return NULL;
}

// [English] Expands macro invocations in a source line recursively.
// Handles string/char literals (skips macro expansion inside them),
// comments, and object-like macros. Recursion depth is limited to 16.
// [Portuguese] Expande invocações de macro em uma linha fonte recursivamente.
// Processa literais string/char (pula expansão de macro dentro deles),
// comentários e macros do tipo objeto. Profundidade de recursão limitada a 16.
static void expand_line(const char *line, FILE *out, int depth)
{
    if (depth > 16)
    {
        fprintf(out, "%s\n", line);
        return;
    }
    const char *p = line;
    char word[MACRO_NAME];
    int wi = 0;
    while (*p)
    {
        // String literal / Literal string
        if (*p == '"')
        {
            if (wi > 0)
            {
                word[wi] = 0;
                macro_t *m = find_macro(word);
                if (m && m->nargs < 0)
                    expand_line(m->value, out, depth + 1);
                else
                    fprintf(out, "%s", word);
                wi = 0;
            }
            fputc(*p, out);
            p++;
            while (*p && *p != '"')
            {
                fputc(*p, out);
                p++;
            }
            if (*p)
            {
                fputc(*p, out);
                p++;
            }
            continue;
        }

        // Char literal / Literal caractere
        if (*p == '\'')
        {
            if (wi > 0)
            {
                word[wi] = 0;
                macro_t *m = find_macro(word);
                if (m && m->nargs < 0)
                    expand_line(m->value, out, depth + 1);
                else
                    fprintf(out, "%s", word);
                wi = 0;
            }
            fputc(*p, out);
            p++;
            while (*p && *p != '\'')
            {
                fputc(*p, out);
                p++;
            }
            if (*p)
            {
                fputc(*p, out);
                p++;
            }
            continue;
        }

        // Whitespace or punctuation / Espaço em branco ou pontuação
        if (*p <= ' ' || *p == ',' || *p == '(' || *p == ')' || *p == ';' || *p == '{' || *p == '}' || *p == '[' || *p == ']')
        {
            if (wi > 0)
            {
                word[wi] = 0;
                macro_t *m = find_macro(word);
                if (m && m->nargs < 0)
                    expand_line(m->value, out, depth + 1);
                else
                    fprintf(out, "%s", word);
                wi = 0;
            }
            fputc(*p, out);
            p++;
        }

        // Block comment / Comentário de bloco
        else if (*p == '/' && *(p + 1) == '*')
        {
            if (wi > 0)
            {
                word[wi] = 0;
                macro_t *m = find_macro(word);
                if (m && m->nargs < 0)
                    expand_line(m->value, out, depth + 1);
                else
                    fprintf(out, "%s", word);
                wi = 0;
            }
            while (*p)
            {
                fputc(*p, out);
                if (*p == '*' && *(p + 1) == '/')
                {
                    fputc(*(p + 1), out);
                    p += 2;
                    break;
                }
                p++;
            }
        }

        // Line comment / Comentário de linha
        else if (*p == '/' && *(p + 1) == '/')
        {
            if (wi > 0)
            {
                word[wi] = 0;
                macro_t *m = find_macro(word);
                if (m && m->nargs < 0)
                    expand_line(m->value, out, depth + 1);
                else
                    fprintf(out, "%s", word);
                wi = 0;
            }
            while (*p)
            {
                fputc(*p, out);
                p++;
            }
            break;
        }

        // Accumulate word character / Acumula caractere de palavra
        else
        {
            if (wi < MACRO_NAME - 1)
                word[wi++] = *p;
            p++;
        }
    }

    // Flush remaining word / Descarrega palavra restante
    if (wi > 0)
    {
        word[wi] = 0;
        macro_t *m = find_macro(word);
        if (m && m->nargs < 0)
            expand_line(m->value, out, depth + 1);
        else
            fprintf(out, "%s", word);
    }
}

// [English] Evaluates a conditional expression (#ifdef/#ifndef).
// Returns 1 if the referenced macro is defined, 0 otherwise.
// [Portuguese] Avalia uma expressão condicional (#ifdef/#ifndef).
// Retorna 1 se a macro referenciada estiver definida, 0 caso contrário.
static int eval_cond(const char *expr)
{
    while (*expr == ' ' || *expr == '\t')
        expr++;
    char name[MACRO_NAME];
    int i = 0;
    while (*expr && *expr > ' ' && *expr != '!' && *expr != '(' && *expr != ')' && i < MACRO_NAME - 1)
        name[i++] = *expr++;
    name[i] = 0;
    return find_macro(name) != NULL;
}

// [English] Main preprocessor entry point: reads an input file line by line,
// processes preprocessor directives (#define, #include, #ifdef, #ifndef, #else, #endif),
// expands macros, and writes the result to the output stream.
// [Portuguese] Ponto de entrada principal do pré-processador: lê um arquivo de entrada
// linha por linha, processa diretivas de pré-processador (#define, #include, #ifdef,
// #ifndef, #else, #endif), expande macros e escreve o resultado no fluxo de saída.
void preproc_run(const char *filename, FILE *output, const char *cpu)
{
    FILE *in = fopen(filename, "r");
    if (!in)
    {
        fprintf(stderr, "error: cannot open: %s\n", filename);
        exit(1);
    }

    nmacros = 0;
    cond_sp = 0;
    cond_skip = 0;

    // Define CPU-specific macro / Define macro específica da CPU
    if (cpu)
    {
        char buf[64];
        snprintf(buf, 64, "__%s__", cpu);
        preproc_add_define(buf, "1");
    }

    char line[8192];
    while (fgets(line, sizeof(line), in))
    {
        char *p = line;
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '#')
        {
            p++;
            char cmd[64];
            int ci = 0;
            while (*p > ' ' && ci < 63)
                cmd[ci++] = *p++;
            cmd[ci] = 0;
            while (*p == ' ')
                p++;

            // #define directive / Diretiva #define
            if (!strcmp(cmd, "define"))
            {
                char name[MACRO_NAME];
                int ni = 0;
                while (*p > ' ' && *p != '(' && ni < MACRO_NAME - 1)
                    name[ni++] = *p++;
                name[ni] = 0;
                if (nmacros < MAX_MACROS)
                {
                    strncpy(macros[nmacros].name, name, MACRO_NAME - 1);
                    // Function-like macro / Macro do tipo função
                    if (*p == '(')
                    {
                        p++;
                        int ai = 0;
                        macros[nmacros].nargs = 0;
                        while (*p && *p != ')')
                        {
                            while (*p == ' ' || *p == ',')
                                p++;
                            int an = 0;
                            while (*p && *p != ',' && *p != ')' && an < MACRO_NAME - 1)
                                macros[nmacros].args[ai][an++] = *p++;
                            macros[nmacros].args[ai][an] = 0;
                            if (an > 0)
                                ai++;
                        }
                        macros[nmacros].nargs = ai;
                        if (*p == ')')
                            p++;
                    }
                    else
                    {
                        // Object-like macro / Macro do tipo objeto
                        macros[nmacros].nargs = -1;
                    }
                    // Parse macro value / Analisa valor da macro
                    int vi = 0;
                    while (*p && vi < MACRO_VAL - 1)
                    {
                        if (*p == '/' && *(p + 1) == '/')
                            break;
                        if (*p == '/' && *(p + 1) == '*')
                        {
                            while (*p && !(*p == '*' && *(p + 1) == '/'))
                                p++;
                            if (*p)
                                p += 2;
                            continue;
                        }
                        if (*p == '\r')
                        {
                            p++;
                            continue;
                        }
                        macros[nmacros].value[vi++] = *p++;
                    }
                    macros[nmacros].value[vi] = 0;
                    nmacros++;
                }
            }

            // #include directive / Diretiva #include
            else if (!strcmp(cmd, "include"))
            {
                while (*p == ' ')
                    p++;
                char fname[256];
                int fi = 0;
                int delim = *p;
                if (delim == '"' || delim == '<')
                {
                    p++;
                    while (*p && *p != delim && *p != '>' && fi < 255)
                        fname[fi++] = *p++;
                    fname[fi] = 0;
                }
                // Search in file's directory / Busca no diretório do arquivo
                char fullpath[512];
                const char *slash = strrchr(filename, '/');
                if (slash && delim == '"')
                {
                    int len = slash - filename + 1;
                    snprintf(fullpath, 512, "%.*s%s", len, filename, fname);
                    FILE *tf = fopen(fullpath, "r");
                    if (tf)
                    {
                        fclose(tf);
                        preproc_run(fullpath, output, NULL);
                        continue;
                    }
                }
                // Search in include directories / Busca nos diretórios de inclusão
                int found = 0;
                for (int i = 0; i < nincl; i++)
                {
                    snprintf(fullpath, 512, "%s/%s", incl_dirs[i], fname);
                    FILE *tf = fopen(fullpath, "r");
                    if (tf)
                    {
                        fclose(tf);
                        preproc_run(fullpath, output, NULL);
                        found = 1;
                        break;
                    }
                }
                // Search in current directory / Busca no diretório atual
                if (!found)
                {
                    FILE *tf = fopen(fname, "r");
                    if (tf)
                    {
                        fclose(tf);
                        preproc_run(fname, output, NULL);
                    }
                    else
                    {
                        fprintf(stderr, "error: include not found: %s\n", fname);
                        exit(1);
                    }
                }
            }

            // #ifdef / #ifndef directive / Diretiva #ifdef / #ifndef
            else if (!strcmp(cmd, "ifdef") || !strcmp(cmd, "ifndef"))
            {
                int is_ifndef = !strcmp(cmd, "ifndef");
                while (*p == ' ')
                    p++;
                int defined = eval_cond(p);
                int take = is_ifndef ? !defined : defined;
                if (cond_sp < 64)
                    cond_stack[cond_sp++] = take ? 0 : 1;
            }

            // #else directive / Diretiva #else
            else if (!strcmp(cmd, "else"))
            {
                if (cond_sp > 0)
                    cond_stack[cond_sp - 1] = !cond_stack[cond_sp - 1];
            }

            // #endif directive / Diretiva #endif
            else if (!strcmp(cmd, "endif"))
            {
                if (cond_sp > 0)
                    cond_sp--;
            }

            // Update skip state / Atualiza estado de pulo condicional
            cond_skip = 0;
            for (int i = 0; i < cond_sp; i++)
                if (cond_stack[i])
                    cond_skip = 1;
            continue;
        }

        // Skip lines inside disabled conditional blocks / Pula linhas em blocos condicionais desabilitados
        if (cond_skip)
            continue;
        expand_line(line, output, 0);
    }

    fclose(in);
}

// [English] Adds a directory to the include search path list (up to 32 directories)
// [Portuguese] Adiciona um diretório à lista de caminhos de busca de inclusão (até 32 diretórios)
void preproc_add_include_dir(const char *dir)
{
    if (nincl < 32)
        incl_dirs[nincl++] = strdup(dir);
}
