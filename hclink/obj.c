#include "link.h"


bool obj_read_raw(object_file_t *file, record_t *rec)
{
    fgetpos(file->file, &file->curr_rec);
    fgetpos(file->file, &file->next_rec);
    memset(rec, 0, sizeof(record_t));
    if(fread(&rec->header, 1, sizeof(record_header_t), file->file) < sizeof(record_header_t)) return false;
    if(rec->header.data_size > 0) fread(rec->data, 1, rec->header.data_size, file->file);
    fgetpos(file->file, &file->next_rec);
    return true;
}

void obj_add_raw(char *filename, char *name, fpos_t pos)
{
    object_file_t *obj = malloc(sizeof(object_file_t) + strlen(name));
    memset(obj, 0, sizeof(object_file_t));
    strcpy(obj->name, name);
    obj->file = fopen(filename, "rb");
    obj->pos = pos;
    if(!obj->file) error("error: can't open file: %s\n", obj->name);
    if(_objs_last) _objs_last->next = obj;
    if(!_objs) _objs = obj;
    _objs_last = obj;
}

void obj_add(char *filename)
{
    record_t rec;
    object_file_t *obj = malloc(sizeof(object_file_t) + strlen(filename));
    memset(obj, 0, sizeof(object_file_t));
    strcpy(obj->name, filename);
    obj->file = fopen(obj->name, "rb");
    if(!obj->file) error("error: can't open file: %s\n", obj->name);
    while(obj_read_raw(obj, &rec))
    {
        if(rec.header.type == REC_FILENAME)
        {
            verbose("Add Object File: %s\n", (char*)rec.data);
            obj_add_raw(filename, (char*)rec.data, obj->curr_rec);
        }
        else if((rec.header.type & MASK_REC_TYPE) == REC_CPU)
        {
            if(_cpu == 0) _cpu = rec.header.type;
            else if(_cpu != rec.header.type && !_multicpu) error("mismatch cpu type at file: %s", filename);
        }
    }
    fclose(obj->file);
    free(obj);
}

bool obj_read(object_file_t *file, record_t *rec)
{
    if(!obj_read_raw(file, rec)) return false;
    if(rec->header.type == REC_END_OF_FILE)
    {
        // reset position to END OF FILE record
        fsetpos(file->file, &file->curr_rec);
        return false;
    }
    return true;
}

void obj_reset(object_file_t *file)
{
    fsetpos(file->file, &file->pos);
}