#include "bcomp.h"

#define MAX_MACROS 256
#define MACRO_NAME 64
#define MACRO_VAL 4096

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

void preproc_add_define(const char *name, const char *value)
{
    if (nmacros >= MAX_MACROS)
        return;
    strncpy(macros[nmacros].name, name, MACRO_NAME - 1);
    strncpy(macros[nmacros].value, value, MACRO_VAL - 1);
    macros[nmacros].nargs = -1;
    nmacros++;
}

static macro_t *find_macro(const char *name)
{
    for (int i = 0; i < nmacros; i++)
        if (!strcmp(macros[i].name, name))
            return &macros[i];
    return NULL;
}

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
        if (*p <= ' ' || *p == ',' || *p == '(' || *p == ')' || *p == ';' || *p == '{' || *p == '}')
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
        else
        {
            if (wi < MACRO_NAME - 1)
                word[wi++] = *p;
            p++;
        }
    }
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
                        macros[nmacros].nargs = -1;
                    }
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
            else if (!strcmp(cmd, "else"))
            {
                if (cond_sp > 0)
                    cond_stack[cond_sp - 1] = !cond_stack[cond_sp - 1];
            }
            else if (!strcmp(cmd, "endif"))
            {
                if (cond_sp > 0)
                    cond_sp--;
            }
            cond_skip = 0;
            for (int i = 0; i < cond_sp; i++)
                if (cond_stack[i])
                    cond_skip = 1;
            continue;
        }

        if (cond_skip)
            continue;
        expand_line(line, output, 0);
    }

    fclose(in);
}

void preproc_add_include_dir(const char *dir)
{
    if (nincl < 32)
        incl_dirs[nincl++] = strdup(dir);
}
