#include "retrolang.h"

static datatype_t *_list = NULL;

void datatype_calcsize(datatype_t *datatype)
{
    switch (datatype->nativetype)
    {
        case NATIVETYPE_8BITS:
            datatype->size = 1;
            return;
        case NATIVETYPE_16BITS:
            datatype->size = 2;
            return;
        case NATIVETYPE_24BITS:
            datatype->size = 3;
            return;
        case NATIVETYPE_32BITS:
            datatype->size = 4;
            return;
        case NATIVETYPE_STRUCTURE:;
            datatype_t *field = datatype->fields;
            datatype->size = 0;
            while(field)
            {
                datatype_calcsize(datatype);
                datatype->size += field->size;
                field = field->next;
            }
            return;
        default:
            error("Invalid nativetype of datatype %s.", datatype->name);
            break;
    }
}

datatype_t *datatype_add(char *name, nativetype_t nativetype, bool is_signed)
{
    datatype_t *obj = malloc(sizeof(datatype_t) + strlen(name));
    if(!obj) error("Datatype memory overflow.");
    memset(obj, 0, sizeof(datatype_t));
    obj->nativetype = nativetype;
    obj->is_signed = is_signed;
    obj->size = -1; // default uncalculated size
    obj->next = _list;
    _list = obj;
    strcpy(obj->name, name);
    return obj;
}

datatype_t *datatype_add_structure(char *name)
{
    return datatype_add(name, NATIVETYPE_STRUCTURE, false);
}

datatype_t *datatype_add_field(datatype_t *structure, char *name, nativetype_t nativetype, bool is_signed, int offset)
{
    datatype_t *obj = malloc(sizeof(datatype_t) + strlen(name));
    if(!obj) error("Datatype memory overflow.");
    if(!structure) error("Structure not defined.");
    datatype_t *field = structure->fields;
    while(field)
    {
        if(field->next == NULL) break;
        field = field->next;
    }
    if(field) field->next = obj;
    else structure->fields = obj;
    memset(obj, 0, sizeof(datatype_t));
    obj->nativetype = nativetype;
    obj->is_signed = is_signed;
    obj->size = -1; // default uncalculated size
    obj->offset = offset;
    strcpy(obj->name, name);
    return obj;
}

datatype_t *datatype_find(char *name)
{
    datatype_t *obj = _list;
    while(obj)
    {
        if(!strcmp(name, obj->name)) break;
        obj = obj->next;
    }
    return NULL;
}

datatype_t *datatype_find_field(datatype_t *structure, char *name)
{
    datatype_t *obj = structure->fields;
    while(obj)
    {
        if(!strcmp(name, obj->name)) break;
        obj = obj->next;
    }
    return NULL;
}