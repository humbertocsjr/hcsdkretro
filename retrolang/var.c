#include "retrolang.h"

var_t *_global_list = NULL;

var_t *var_find_global(char *name)
{
    var_t *obj = _global_list;
    while(obj)
    {
        if(!strcmp(obj->name, name)) return obj;
        obj = obj->next;
    }
    return NULL;
}

var_t *var_add_global(char *name, datatype_t *datatype, bool is_array, uint16_t array_size, bool is_pointer, uint8_t pointer_level)
{
    var_t *obj = malloc(sizeof(var_t) + strlen(name));
    if(!obj) error("Variable memory overflow.");
    memset(obj, 0, sizeof(var_t));
    obj->datatype = datatype;
    obj->is_array = is_array;
    obj->is_global = true;
    obj->array_size = array_size;
    obj->is_pointer = is_pointer;
    obj->pointer_level = pointer_level;
    obj->local_offset = 0;
    obj->next = _global_list;
    _global_list = obj;
    strcpy(obj->name, name);
    return obj;
}