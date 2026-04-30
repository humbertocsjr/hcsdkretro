#include "build.h"

section_t *_section = NULL;

char *ltrim(char *s) 
{
    while(isspace((unsigned char)*s)) 
    {
        s++;
    }
    return s;
}

char *rtrim(char *s) 
{
    if(!s) return s;
    char *back = s + strlen(s) - 1;
    while(back >= s && isspace((unsigned char)*back)) 
    {
        back--;
    }
    if(*s == 0) return s;
    *(back+1) = '\0';
    return s;
}

section_t *get_section(char *name, char *subsection)
{
    section_t *section = _section;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            return section;
        }
        section = section->next;
    }
    return NULL;
}

section_t *add_section(char *name, char *subsection)
{
    char *tmp;
    section_t *section = _section;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            return section;
        }
        section = section->next;
    }
    section = malloc(sizeof(section_t));
    section->next = _section;
    _section = section;
    tmp = rtrim(ltrim(name));
    section->name = malloc(strlen(tmp) + 1);
    strcpy(section->name, tmp);
    tmp = rtrim(ltrim(subsection));
    section->subsection = malloc(strlen(tmp) + 1);
    strcpy(section->subsection, tmp);
    section->keys = NULL;
    return section;
}

char *get_value(char *name, char *subsection, char *key)
{
    section_t *section = _section;
    keyvalue_t *kv;
    while(section)
    {
        if(!strcmp(name, section->name) && !strcmp(subsection, section->subsection))
        {
            kv = section->keys;
            while(kv)
            {
                if(!strcmp(key, kv->key))
                {
                    return kv->value;
                }
                kv = kv->next;
            }
        }
        section = section->next;
    }
    return "";
}

bool get_value_bool(char *name, char *subsection, char *key)
{
    char *value = get_value(name, subsection, key);
    char *ptr = value;
    while(*ptr)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }
    if(!strcmp(value, "yes") || !strcmp(value, "true") || !strcmp(value, "1") || !strcmp(value, "enable")) return true;
    return false;
}

void set_key(section_t *section, char *key, char *value)
{
    char *tmp;
    if(!section)
    {
        fprintf(stderr, "error: can't add key without section: %s\n", key);
        exit(1);
    }
    keyvalue_t *kv = section->keys;
    while(kv)
    {
        if(!strcmp(key, kv->key))
        {
            if(kv->value) free(kv->value);
            kv->value = malloc(strlen(value) + 1);
            strcpy(kv->value, value);
            return;
        }
        kv = kv->next;
    }
    kv = malloc(sizeof(keyvalue_t));
    tmp = rtrim(ltrim(key));
    kv->key = malloc(strlen(tmp) + 1);
    strcpy(kv->key, tmp);
    tmp = rtrim(ltrim(value));
    kv->value = malloc(strlen(tmp) + 1);
    strcpy(kv->value, tmp);
    kv->next = section->keys;
    section->keys = kv;
}



void cfg_process(char *filename)
{
    char key[1024];
    char value[1024];
    int key_i = 0;
    int value_i = 0;
    int step = 0;
    int c = 0;
    section_t *section = NULL;
    FILE *cfg = fopen(filename, "r");
    if(!cfg)
    {
        fprintf(stderr, "error: can't open file: %s\n", filename);
        exit(1);
    }
    while(1)
    {
        c = fgetc(cfg);
        if(c == EOF) break;
        if(key_i >= 1023)
        {
            fprintf(stderr, "error: key size overflow: %s\n", key);
            exit(1);
        }
        if(value_i >= 1023)
        {
            fprintf(stderr, "error: value size overflow: %s = %s\n", key, value);
            exit(1);
        }
        if(c == ';' || c == '#')
        {
            while(c != '\n') c = fgetc(cfg);
        }
        switch (step)
        {
            case 0:
                {
                    strcpy(key, "");
                    strcpy(value, "");
                    key_i = 0;
                    value_i = 0;
                    if(c == '[')
                    {
                        step = 1;
                    }
                    else if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
                    {
                        step = 3;
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            case 1: // section name
                {
                    if(c == ':')
                    {
                        step = 2;
                    }
                    else if(c == ']')
                    {
                        step = 0;
                        section = add_section(key, "");
                    }
                    else
                    {
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            case 2: // subsection
                {
                    if(c == ']')
                    {
                        step = 0;
                        section = add_section(key, value);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                }
                break;
            case 3: // key
                {
                    if(c == '=')
                    {
                        step = 4;
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        step = 0;
                        set_key(section, key, "");
                    }
                    else
                    {
                        key[key_i++] = c;
                        key[key_i] = 0;
                    }
                }
                break;
            case 4: // value
                {
                    if(c == '"')
                    {
                        step = 5;
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        step = 0;
                        set_key(section, key, value);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                    
                }
                break;
            case 5: // quoted value
                {
                    if(c == '"')
                    {
                        step = 0;
                        set_key(section, key, value);
                    }
                    else if(c == '\n' || c == '\r' || c == EOF)
                    {
                        fprintf(stderr, "error: unterminated quoted value\n");
                        exit(1);
                    }
                    else
                    {
                        value[value_i++] = c;
                        value[value_i] = 0;
                    }
                }
                break;
        }
    }
    if(step == 1 || step == 2)
    {
        add_section(key, value);
    }
    else if(step == 3 || step == 4 || step == 5)
    {
        set_key(section, key, value);
    }
    fclose(cfg);
}