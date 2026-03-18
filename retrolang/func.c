#include "retrolang.h"

static func_t *_list = NULL;

func_t *func_get_list()
{
    return _list;
}

func_t *func_global()
{
    func_t *obj = func_find("__GLOBAL__");
    if(!obj) obj = func_add("__GLOBAL__");
    obj->is_global_context = true;
    return obj;
}

func_t *func_find(char *name)
{
    func_t *obj = _list;
    while(obj)
    {
        if(!strcmp(obj->name, name)) return obj;
        obj = obj->next;
    }
    return NULL;
}

func_t *func_add(char *name)
{
    func_t *obj = malloc(sizeof(func_t) + strlen(name));
    if(!obj) error("Function memory overflow.");
    memset(obj, 0, sizeof(func_t));
    strcpy(obj->name, name);
    obj->next = _list;
    _list = obj;
    return obj;
}

var_t *func_find_var(func_t *func, char *name)
{
    var_t *obj = func->args;
    while(obj)
    {
        if(!strcmp(obj->name, name)) return obj;
        obj = obj->next;
    }
    obj = func->vars;
    while(obj)
    {
        if(!strcmp(obj->name, name)) return obj;
        obj = obj->next;
    }
    return NULL;
}

var_t *func_add_var(func_t *func, char *name, datatype_t *datatype, bool is_array, uint16_t array_size, bool is_pointer, uint8_t pointer_level)
{
    var_t *obj = malloc(sizeof(var_t) + strlen(name));
    if(!obj) error("Variable memory overflow.");
    memset(obj, 0, sizeof(var_t));
    obj->datatype = datatype;
    obj->is_array = is_array;
    obj->is_global = func->is_global_context;
    obj->array_size = array_size;
    obj->is_pointer = is_pointer;
    obj->pointer_level = pointer_level;
    obj->local_offset = 0;
    obj->next = func->vars;
    func->vars = obj;
    strcpy(obj->name, name);
    return obj;
}

var_t *func_add_arg(func_t *func, char *name, datatype_t *datatype, bool is_pointer, uint8_t pointer_level)
{
    var_t *obj = malloc(sizeof(var_t) + strlen(name));
    if(!obj) error("Variable memory overflow.");
    memset(obj, 0, sizeof(var_t));
    obj->datatype = datatype;
    obj->is_array = false;
    obj->is_global = false;
    obj->array_size = 0;
    obj->is_pointer = is_pointer;
    obj->pointer_level = pointer_level;
    obj->local_offset = 0;
    obj->next = func->args;
    func->args = obj;
    strcpy(obj->name, name);
    return obj;
}