#include "retrolang.h"

var_t *var_find_global(char *name)
{
    var_t *obj = func_global()->vars;
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
    obj->next = func_global()->vars;
    func_global()->vars = obj;
    strcpy(obj->name, name);
    return obj;
}

datatype_t * var_calc_datatype(var_t *v)
{
    datatype_t *dt = v->datatype;
    if(v->is_pointer)
    {
        dt = datatype_find("pointer");
        if(!dt) error("ponter datatype is missing in this cpu");
    }
    datatype_calcsize(dt);
    return dt;
}

int32_t var_calc_total_size(var_t *v)
{
    datatype_t *dt = var_calc_datatype(v);
    if(v->is_array)
    {
        return v->array_size * dt->size;
    }
    return dt->size;
}

int32_t var_calc_item_size(var_t *v)
{
    datatype_t *dt = var_calc_datatype(v);
    return dt->size;
}
