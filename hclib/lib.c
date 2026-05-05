#include "lib.h"

FILE *_lib = NULL;
FILE *_tmp = NULL;
object_t *_objs = NULL;
object_t *_last_obj = NULL;

// [English] Display help
// [Portuguese] Exibe ajuda
void help()
{
    printf("HC Librarian for Retro Computing v%d.%d R%d\n", VERSION, SUBVERSION, REVISION);
    printf("HC Software Development Kit for Retro Computing\n");
    printf("Copyright (c) 2025,2026 Humberto Costa dos Santos Junior\n\n");
    printf("Usage: hclib [LIBRARY FILE] [OBJECT FILES ...]\n\n");
    printf("Auto replace/add objects to library\n");
    printf("Create temporary file named hclib.$$$\n");
    exit(1);
}

// [English] Emit error and cleanup
// [Portuguese] Emite erro e faz limpeza
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

// [English] Read object record from file
// [Portuguese] Le um registro de objeto do arquivo
bool obj_read(FILE *file, record_t *rec)
{
    memset(rec, 0, sizeof(record_t));
    if(fread(&rec->header, 1, sizeof(record_header_t), file) < sizeof(record_header_t)) return false;
    if(rec->header.data_size > 0) fread(rec->data, 1, rec->header.data_size, file);
    return true;
}

// [English] Write object record to file
// [Portuguese] Escreve um registro de objeto no arquivo
bool obj_write(FILE *file, record_t *rec)
{
    fwrite(&rec->header, 1, sizeof(record_header_t), file);
    if(rec->header.data_size > 0) fwrite(rec->data, 1, rec->header.data_size, file);
    return true;
}

// [English] Extract basename from path
// [Portuguese] Extrai o nome base do caminho
static const char *obj_basename(const char *path)
{
    const char *p = path;
    while(*p) p++;
    while(p > path && p[-1] != '/' && p[-1] != '\\') p--;
    return p;
}

// [English] Check if object already exists in list
// [Portuguese] Verifica se o objeto ja existe na lista
bool obj_exists(char *name)
{
    const char *base = obj_basename(name);
    object_t *obj = _objs;
    while(obj)
    {
        if(!strcmp(base, obj_basename(obj->name)))
        {
            return true;
        }
        obj = obj->next;
    }
    return false;
}

// [English] Entry point
// [Portuguese] Ponto de entrada
// [English] Reads input objects, merges with existing library, writes output
// [Portuguese] Le objetos de entrada, mescla com biblioteca existente, escreve saida
int main(int argc, char **argv)
{
    char *out_name = NULL;
    record_t rec;

    // [English] Parse output filename
    // [Portuguese] Analisa nome do arquivo de saida
    if(argc > 1 && strcmp(argv[1], "-h")) out_name = argv[1];
    else help();

    // [English] Open temporary and existing library
    // [Portuguese] Abre temporario e biblioteca existente
    _tmp = fopen("hclib.$$$", "wb");
    _lib = fopen(out_name, "rb");

    // [English] Process input object files
    // [Portuguese] Processa arquivos-objeto de entrada
    for(int i = 2; i < argc; i++)
    {
        FILE *obj = fopen(argv[i], "rb");
        if(!obj) error("can't open object file: %s", argv[i]);
        bool has_filename = false;

        // [English] Read all records from object
        // [Portuguese] Le todos os registros do objeto
        while(obj_read(obj, &rec))
        {
            switch (rec.header.type)
            {
                case REC_FILENAME:
                    has_filename = rec.header.data_size > 0 && !obj_exists((char*)rec.data);
                    if(has_filename)
                    {
                        obj_write(_tmp, &rec);
                        object_t *o = malloc(sizeof(object_t) + rec.header.data_size);
                        memset(o, 0, sizeof(object_t) + rec.header.data_size);
                        strcpy(o->name, (char*)rec.data);
                        if(_last_obj)
                        {
                            _last_obj->next = o;
                            _last_obj = o;
                        }
                        else
                        {
                            o->next = _objs;
                            _objs = o;
                            _last_obj = o;
                        }
                    }
                    break;
                default:
                    if(has_filename) obj_write(_tmp, &rec);
                    break;
            }
        }
        fclose(obj);
    }

    // [English] Merge existing library (skip replaced objects)
    // [Portuguese] Mescla biblioteca existente (pula objetos substituidos)
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

    // [English] Finalize: close temp, replace original
    // [Portuguese] Finaliza: fecha temp, substitui original
    fclose(_tmp);
    remove(out_name);
    rename("hclib.$$$", out_name);

    return 0;
}
