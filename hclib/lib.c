#include "lib.h"

FILE *_lib = NULL;
FILE *_tmp = NULL;
object_t *_objs = NULL;

void help()
{
    printf("HC Librarian for Retro Computing v%d.%d R%d\n", VERISION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hclib [LIBRARY FILE] [OBJECT FILES ...]\n\n");
    printf("Auto replace/add objects to library\n");
    printf("Create temporary file named hclib.$$$\n");
    exit(1);
}

void error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    if(_tmp) fclose(_tmp);
    if(_lib) fclose(_lib);
    remove("hclib.$$$");
    exit(1);
}

bool obj_read(FILE *file, record_t *rec)
{
    memset(rec, 0, sizeof(record_t));
    if(fread(&rec->header, 1, sizeof(record_header_t), file) < sizeof(record_header_t)) return false;
    if(rec->header.data_size > 0) fread(rec->data, 1, rec->header.data_size, file);
    return true;
}

bool obj_write(FILE *file, record_t *rec)
{
    fwrite(&rec->header, 1, sizeof(record_header_t), file);
    if(rec->header.data_size > 0) fwrite(rec->data, 1, rec->header.data_size, file);
    return true;
}

bool obj_exists(char *name)
{
    object_t *obj = _objs;
    while(obj)
    {
        if(!strcmp(name, obj->name))
        {
            return true;
        }
        obj = obj->next;
    }
    return false;
}

int main(int argc, char **argv)
{
    char *out_name = NULL;
    record_t rec;
    if(argc > 1 && strcmp(argv[1], "-h")) out_name = argv[1];
    else help();
    _tmp = fopen("hclib.$$$", "wb");
    _lib = fopen(out_name, "rb");
    for(int i = 2; i < argc; i++)
    {
        FILE *obj = fopen(argv[i], "rb");
        if(!obj) error("can't open object file: %s", argv[i]);
        while(obj_read(obj, &rec))
        {
            switch (rec.header.type)
            {
                case REC_FILENAME:
                    if(rec.header.data_size)
                    {
                        obj_write(_tmp, &rec);
                        if(obj_exists((char*)rec.data)) error("object already exists: %s", (char *)rec.data);
                        object_t *obj = malloc(sizeof(object_t) + rec.header.data_size);
                        memset(obj, 0, sizeof(object_t) + rec.header.data_size);
                        obj->next = _objs;
                        strcpy(obj->name, (char*)rec.data);
                        _objs = obj;
                    }
                    break;
                default:
                    obj_write(_tmp, &rec);
                    break;
            }
        }
        fclose(obj);
    }
    if(_lib)
    {
        while(obj_read(_lib, &rec))
        {
            switch (rec.header.type)
            {
                case REC_FILENAME:
                    if(obj_exists((char*)rec.data))
                    {
                        while(obj_read(_lib, &rec))
                        {
                            if(rec.header.type == REC_END_OF_FILE) break;
                        }
                    }
                    else obj_write(_tmp, &rec);
                    break;
                default:
                    obj_write(_tmp, &rec);
                    break;
            }
        }
        fclose(_lib);
    }
    fclose(_tmp);
    remove(out_name);
    rename("hclib.$$$", out_name);
    return 0;
}

