#include "build.h"

#ifdef WINDOWS_HOST
#define PATHSEPARATOR '\\'
#define PATHSEPARATORSTR "\\"
#else
#define PATHSEPARATOR '/'
#define PATHSEPARATORSTR "/"
#endif

obj_t *_objs = NULL;
obj_t *_last_obj = NULL;
char *_path = "";

void remove_ext(char *filename) 
{
    char *last_dot = strrchr(filename, '.');
    if (last_dot != NULL && last_dot != filename) {
        *last_dot = '\0';
    }
}

char *get_ext(char *filename) 
{
    char *last_dot = strrchr(filename, '.');
    if (last_dot != NULL && last_dot != filename) 
    {
        return last_dot;
    }
    return "";
}

void help()
{
    printf("HC Builder for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hcbuild [PROJECT FILE] [COMMAND] [CONFIGURATION NAME (default: release)]\n");
    printf("Commands:\n");
    printf(" make           : Make project configuration\n");
    printf(" clean          : Clean project output files\n");
    exit(1);
}

void make_files(section_t *section)
{
    int st;
    char cmd[8000];
    char source_name[2048];
    char obj_name[2048];
    char dump_name[2048];
    char *sdk_path = get_value("config", "", "sdk_path");

    if(!section) return;
    keyvalue_t *kv = section->keys;
    while(kv)
    {
        if(kv->key[0] != PATHSEPARATOR)
        {
            strcpy(source_name, _path);
            if(*_path && _path[strlen(_path)-1] != PATHSEPARATOR) strcat(source_name, PATHSEPARATORSTR);
            strncat(source_name, kv->key, 1000);
        }
        else strncpy(source_name, kv->key, 1000);
        strcpy(obj_name, source_name);
        remove_ext(obj_name);
        strcat(obj_name, ".obj");
        strcpy(dump_name, source_name);
        remove_ext(dump_name);
        strcat(dump_name, ".dump");
        if(!strcmp(get_ext(source_name), ".s") || !strcmp(get_ext(source_name), ".S"))
        {
            strcpy(cmd, sdk_path);
            if(strlen(sdk_path) && sdk_path[strlen(sdk_path)-1] != PATHSEPARATOR) strcat(cmd, PATHSEPARATORSTR);
            #ifdef WINDOWS_HOST
            strcat(cmd, "hcasm-");
            strcat(cmd, section->subsection);
            strcat(cmd, ".exe");
            #else
            strcat(cmd, "hcasm-");
            strcat(cmd, section->subsection);
            #endif
            strcat(cmd, " -o ");
            strcat(cmd, obj_name);
            if(get_value_bool("config", "", "dump"))
            {
                strcat(cmd, " -dump ");
                strcat(cmd, dump_name);
            }
            strcat(cmd, " ");
            strcat(cmd, source_name);
            if(get_value_bool("config", "", "verbose")) printf("%s\n", cmd);
            st = system(cmd);
            if(st) exit(st);
            obj_t *obj = malloc(sizeof(obj_t) + strlen(obj_name));
            strcpy(obj->name, obj_name);
            obj->next = NULL;
            if(_last_obj)
            {
                _last_obj->next = obj;
            }
            else
            {
                _objs = obj;
            }
            _last_obj = obj;
        }
        else
        {
            fprintf(stderr, "error: extension not supported: %s\n", source_name);
            exit(1);
        }
        kv = kv->next;
    }
}

void clean_files(section_t *section)
{
    char obj_name[1024];
    if(!section) return;
    keyvalue_t *kv = section->keys;
    while(kv)
    {
        strncpy(obj_name, kv->key, 1000);
        remove_ext(obj_name);
        strcat(obj_name, ".obj");
        remove(obj_name);
        if(get_value_bool("config", "", "dump"))
        {
            remove_ext(obj_name);
            strcat(obj_name, ".dump");
            remove(obj_name);
        }
        kv = kv->next;
    }
}

void make_link(section_t *section, char *config)
{
    int st;
    char *cmd;
    char *out_file = NULL;
    char *sym_file = NULL;
    char *format = NULL;
    char *text_offset = NULL;
    char *data_offset = NULL;
    char *bss_offset = NULL;
    char *align = NULL;
    char *sdk_path = get_value("config", "", "sdk_path");
    if(!section)
    {
        fprintf(stderr, "error: link configuration not found: %s\n", config);
        exit(1);
    }
    out_file = get_value(section->name, section->subsection, "filename");
    sym_file = get_value(section->name, section->subsection, "symbols");
    format = get_value(section->name, section->subsection, "format");
    text_offset = get_value(section->name, section->subsection, "text");
    data_offset = get_value(section->name, section->subsection, "data");
    bss_offset = get_value(section->name, section->subsection, "bss");
    align = get_value(section->name, section->subsection, "align");
    if(strlen(format) == 0) format = "bin";
    if(strlen(out_file) == 0) out_file = "a.out";
    obj_t *obj = _objs;
    size_t cmd_size = 256 + strlen(out_file) + strlen(_path) + strlen(sym_file) + strlen(sdk_path);
    while(obj)
    {
        cmd_size += 2 + strlen(obj->name);
        obj = obj->next;
    }
    cmd = malloc(cmd_size);
    if(!strcmp(format, "lib"))
    {
        strcpy(cmd, sdk_path);
        if(strlen(sdk_path) && sdk_path[strlen(sdk_path)-1] != PATHSEPARATOR) strcat(cmd, PATHSEPARATORSTR);
        #ifdef WINDOWS_HOST
        strcat(cmd, "hclib.exe ");
        #else
        strcat(cmd, "hclib ");
        #endif
        if(out_file[0] != PATHSEPARATOR)
        {
            strcat(cmd, _path);
            if(*_path && _path[strlen(_path)-1] != PATHSEPARATOR) strcat(cmd, PATHSEPARATORSTR);
        }
        strcat(cmd, out_file);
    }
    else
    {
        strcpy(cmd, sdk_path);
        if(strlen(sdk_path) && sdk_path[strlen(sdk_path)-1] != PATHSEPARATOR) strcat(cmd, PATHSEPARATORSTR);
        #ifdef WINDOWS_HOST
        strcat(cmd, "hclink-");
        strcat(cmd, format);
        strcat(cmd, ".exe");
        #else
        strcat(cmd, "hclink-");
        strcat(cmd, format);
        #endif
        strcat(cmd, " -o ");
        if(out_file[0] != PATHSEPARATOR)
        {
            strcat(cmd, _path);
            if(*_path && _path[strlen(_path)-1] != PATHSEPARATOR) strcat(cmd, PATHSEPARATORSTR);
        }
        strcat(cmd, out_file);
        if(strlen(text_offset))
        {
            strcat(cmd, " -text ");
            strcat(cmd, text_offset);
        }
        if(strlen(data_offset))
        {
            strcat(cmd, " -data ");
            strcat(cmd, data_offset);
        }
        if(strlen(bss_offset))
        {
            strcat(cmd, " -bss ");
            strcat(cmd, bss_offset);
        }
        if(strlen(align))
        {
            strcat(cmd, " -align ");
            strcat(cmd, align);
        }
        if(strlen(sym_file))
        {
            strcat(cmd, " -sym ");
            strcat(cmd, sym_file);
        }
    }
    obj = _objs;
    while(obj)
    {
        strcat(cmd, " ");
        strcat(cmd, obj->name);
        obj = obj->next;
    }
    if(get_value_bool("config", "", "verbose")) printf("%s\n", cmd);
    st = system(cmd);
    if(st) exit(st);
}

void clean_link(section_t *section, char *config)
{
    char cmd[8000];
    char *out_file = NULL;
    if(!section)
    {
        fprintf(stderr, "error: link configuration not found: %s\n", config);
        exit(1);
    }
    out_file = get_value(section->name, section->subsection, "filename");
    if(strlen(out_file) == 0) out_file = "a.out";
    remove(out_file);
    out_file = get_value(section->name, section->subsection, "symbols");
    if(strlen(out_file)) remove(out_file);

}

void make_libs(section_t *section)
{
    char obj_name[2048];
    if(!section) return;
    keyvalue_t *kv = section->keys;
    while(kv)
    {
        if(kv->key[0] != PATHSEPARATOR)
        {
            strcpy(obj_name, _path);
            if(*_path && _path[strlen(_path)-1] != PATHSEPARATOR) strcat(obj_name, PATHSEPARATORSTR);
            strncat(obj_name, kv->key, 1000);
        }
        else strncpy(obj_name, kv->key, 1000);
        obj_t *obj = malloc(sizeof(obj_t) + strlen(obj_name));
        strcpy(obj->name, obj_name);
        obj->next = NULL;
        if(_last_obj)
        {
            _last_obj->next = obj;
        }
        else
        {
            _objs = obj;
        }
        _last_obj = obj;
        kv = kv->next;
    }

}

int main(int argc, char **argv)
{
    section_t *section = NULL;
    if(argc != 4) help();
    cfg_process(argv[1]);
    _path = strdup(argv[1]);
    _path = dirname(_path);
    if(!strcmp(argv[2], "make"))
    {
        make_libs(get_section("lib", "start"));
        make_libs(get_section("libs", "start"));
        make_files(get_section("files", "8080"));
        make_files(get_section("files", "8085"));
        make_files(get_section("files", "8086"));
        make_files(get_section("files", "z80"));
        make_libs(get_section("lib", ""));
        make_libs(get_section("libs", ""));
        make_link(get_section("link", argv[3]), argv[3]);
    }
    else if(!strcmp(argv[2], "clean"))
    {
        clean_files(get_section("files", "8080"));
        clean_files(get_section("files", "8085"));
        clean_files(get_section("files", "8086"));
        clean_files(get_section("files", "z80"));
        clean_link(get_section("link", argv[3]), argv[3]);
    }
    else 
    {
        fprintf(stderr, "error: hcbuild command unknown: %s\n", argv[2]);
        return 1;
    }
    return 0;
}