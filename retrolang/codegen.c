#include "retrolang.h"

int _label = 0;

void codegen_command(func_t *func, command_t *cmd);
void codegen_expr(func_t *func, command_t *cmd, expr_t *e, datatype_t *dt, bool to_aux_reg);

int new_label()
{
    return _label++;
}

void codegen_comment(char *fmt, ...)
{
    out_inline("%s", cpu_comment_start());
    va_list args;
    va_start(args, fmt);
    out_inline_vargs(fmt, args);
    va_end(args);
    out_line("%s", cpu_comment_end());
}

void codegen_global_vars()
{
    var_t *v = func_global()->vars;
    while(v)
    {
        codegen_comment("GLOBAL VAR %s", v->name);
        cpu_global_variable(v->name, var_calc_total_size(v->error_reference, v));
        v = v->next;
    }
}

datatype_t *codegen_avail_expr_type(func_t *func, command_t *cmd, expr_t *e, datatype_t *dt)
{
    var_t *v;
    switch(e->tok)
    {
        case ACT_POINTER_SYMBOL:
            v = func_find_var(func, e->token);
            if(!v) v = var_find_global(e->token);
            if(!v) error("variable not found: %s", e->token);
            return v->pointer_level != e->value ? datatype_find("pointer") : v->datatype;
            break;
        case TOK_STRING:
            return datatype_find("pointer");
        case TOK_SYMBOL:
            v = func_find_var(func, e->token);
            if(!v) v = var_find_global(e->token);
            if(!v) error("variable not found: %s", e->token);
            return var_calc_datatype(e, v);
            break;
        default: error("invalid avail expr type: %i[%s]", e->tok, e->token);
    }
    return NULL;
}

void codegen_autoconvert(expr_t *e, datatype_t *dst, datatype_t *src)
{
    switch(dst->nativetype)
    {
        case NATIVETYPE_8BITS:
            cpu_convert_to_8bit(src->nativetype, src->is_signed, dst->is_signed);
            break;
        case NATIVETYPE_16BITS:
            cpu_convert_to_16bit(src->nativetype, src->is_signed, dst->is_signed);
            break;
        case NATIVETYPE_24BITS:
            cpu_convert_to_24bit(src->nativetype, src->is_signed, dst->is_signed);
            break;
        case NATIVETYPE_32BITS:
            cpu_convert_to_32bit(src->nativetype, src->is_signed, dst->is_signed);
            break;
        default:  error_expr(e, "invalid data conversion.");
    }
}

void codegen_write_to_expr(func_t *func, command_t *cmd, expr_t *e, datatype_t *dt)
{
    var_t *v;
    switch(e->tok)
    {
        case ACT_POINTER_SYMBOL:
            v = func_find_var(func, e->token);
            if(!v) v = var_find_global(e->token);
            if(!v) error_expr(e, "variable not found: %s", e->token);
            cpu_push_acc(v->datatype->nativetype);
            if(v->is_global) cpu_load_global_var(var_calc_datatype(e, v)->nativetype, v->name);
            else cpu_load_local_var(var_calc_datatype(e, v)->nativetype, v->name, v->local_offset);
            codegen_autoconvert(e, datatype_find("pointer"), var_calc_datatype(e, v));
            for(int i = 1; i < e->value; i++)
            {
                cpu_load_pointer(datatype_find("pointer")->nativetype);
            }
            cpu_pop_aux(v->datatype->nativetype);
            cpu_store_aux_to_acc_pointer(v->datatype->nativetype);
            break;
        case TOK_SYMBOL:
            v = func_find_var(func, e->token);
            if(!v) v = var_find_global(e->token);
            if(!v) error("variable not found: %s", e->token);
            if(v->is_global)
            {
                cpu_store_global_var(var_calc_datatype(e, v)->nativetype, v->name);
            }
            else
            {
                cpu_store_local_var(var_calc_datatype(e, v)->nativetype, v->name, v->local_offset);
            }
            break;
        default: error("unsupported write to expression type: %i[%s]", e->tok, e->token);
    }
}

int codegen_args(func_t *func, command_t *cmd, expr_t *e, func_t *callee, var_t *args)
{
    int size = 0;
    if(e && callee && args && e->tok == TOK_COMMA)
    { 
        codegen_expr(func, cmd, e->right, var_calc_datatype(e, args), false);
        cpu_push_acc(var_calc_datatype(e, args)->nativetype);
        if(e->left) size += codegen_args(func, cmd, e->left, callee, args->next);
        if(!e->left && args->next) error_expr(e, "argument count mismatch.");
        size += var_calc_total_size(e, args);
    }
    else if(e && callee && args)
    { 
        codegen_expr(func, cmd, e, var_calc_datatype(e, args), false);
        cpu_push_acc(var_calc_datatype(e, args)->nativetype);
        if(args->next) error_expr(e, "argument count mismatch.");
        size += var_calc_total_size(e, args);
    }
    else if(e && callee)
    {
        error_expr(e, "argument count mismatch.");
    }
    else if(e)
    {
        codegen_expr(func, cmd, e->right, datatype_find("int"), false);
        cpu_push_acc(datatype_find("int")->nativetype);
        if(e->left) size += codegen_args(func, cmd, e->left, NULL, NULL);
        size += var_calc_total_size(e, args);
    }
    size += cpu_get_stack_align() - 1;
    size &= ~(cpu_get_stack_align() - 1);
    return size;
}

void codegen_expr(func_t *func, command_t *cmd, expr_t *e, datatype_t *dt, bool to_aux_reg)
{
    int levels = 0;
    var_t *ref_var;
    func_t *ref_func;
    int args_size;
    int lbl1, lbl2;
    bool push_acc = to_aux_reg && (e->tok != TOK_INTEGER && e->tok != TOK_SYMBOL && e->tok != KEY_ADDRESSOF && e->tok != KEY_SIZEOF);
    if(!e) return;
    if(push_acc)
    {
        cpu_push_acc(dt->nativetype);
    }
    switch(e->tok)
    {
        case TOK_STRING:
            if(to_aux_reg) cpu_set_aux_as_string(dt->nativetype, e->token);
            else cpu_set_acc_as_string(dt->nativetype, e->token);
            break;
        case TOK_INTEGER:
            if(to_aux_reg) cpu_set_aux(dt->nativetype, e->value);
            else cpu_set_acc(dt->nativetype, e->value);
            break;
        case TOK_ADD:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_add(dt->nativetype);
            break;
        case TOK_SUB:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_sub(dt->nativetype);
            break;
        case TOK_MUL:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_mul(dt->nativetype, dt->is_signed);
            break;
        case TOK_DIV:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_div(dt->nativetype, dt->is_signed);
            break;
        case TOK_MOD:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_mod(dt->nativetype, dt->is_signed);
            break;
        case TOK_OR:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_or(dt->nativetype);
            break;
        case TOK_AND:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_and(dt->nativetype);
            break;
        case TOK_XOR:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_xor(dt->nativetype);
            break;
        case TOK_EQUAL:
        case TOK_NOT_EQUAL:
        case TOK_GREATER_THAN:
        case TOK_GREATER_OR_EQUAL:
        case TOK_LESS_THAN:
        case TOK_LESS_OR_EQUAL:
            codegen_expr(func, cmd, e->left, dt, false);
            codegen_expr(func, cmd, e->right, dt, true);
            cpu_compare(dt->nativetype, e->tok, dt->is_signed);
            break;
        case TOK_OR_ELSE:
            codegen_expr(func, cmd, e->left, dt, false);
            lbl1 = new_label();
            cpu_jump_if_true(dt->nativetype, lbl1);
            codegen_expr(func, cmd, e->right, dt, false);
            cpu_label(lbl1);
            break;
        case TOK_AND_ALSO:
            codegen_expr(func, cmd, e->left, dt, false);
            lbl1 = new_label();
            cpu_jump_if_false(dt->nativetype, lbl1);
            codegen_expr(func, cmd, e->right, dt, false);
            cpu_label(lbl1);
            break;
        case TOK_ASSIGN:
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case TOK_ADD_ASSIGN:
            codegen_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), true);
            cpu_add(codegen_avail_expr_type(func, cmd, e->left, dt)->nativetype);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case TOK_SUB_ASSIGN:
            codegen_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), true);
            cpu_sub(codegen_avail_expr_type(func, cmd, e->left, dt)->nativetype);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case TOK_MUL_ASSIGN:
            codegen_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), true);
            cpu_mul(codegen_avail_expr_type(func, cmd, e->left, dt)->nativetype, codegen_avail_expr_type(func, cmd, e->left, dt)->is_signed);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case TOK_DIV_ASSIGN:
            codegen_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), true);
            cpu_div(codegen_avail_expr_type(func, cmd, e->left, dt)->nativetype, codegen_avail_expr_type(func, cmd, e->left, dt)->is_signed);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case TOK_MOD_ASSIGN:
            codegen_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt), false);
            codegen_expr(func, cmd, e->right, codegen_avail_expr_type(func, cmd, e->left, dt), true);
            cpu_mod(codegen_avail_expr_type(func, cmd, e->left, dt)->nativetype, codegen_avail_expr_type(func, cmd, e->left, dt)->is_signed);
            codegen_write_to_expr(func, cmd, e->left, codegen_avail_expr_type(func, cmd, e->left, dt));
            break;
        case KEY_ADDRESSOF:
            if(!e->right || e->right->tok != TOK_SYMBOL) error_expr(e, "function/variable expected inside addressof command.");
            ref_var = func_find_var(func, e->right->token);
            if(!ref_var) ref_var = var_find_global(e->right->token);
            if(ref_var)
            {
                if(to_aux_reg)
                {
                    if(ref_var->is_global) cpu_set_aux_as_pointer(dt->nativetype, e->right->token);
                    else cpu_set_aux_pointer_local(dt->nativetype, e->right->token, ref_var->local_offset);
                }
                else
                {
                    if(ref_var->is_global) cpu_set_acc_as_pointer(dt->nativetype, e->right->token);
                    else cpu_set_acc_pointer_local(dt->nativetype, e->right->token, ref_var->local_offset);
                }
            }
            else
            {
                ref_func = func_find(e->right->token);
                if(!ref_func) error_expr(e, "function/variable not found: %s", e->right->token);

                if(to_aux_reg)
                {
                    cpu_set_aux_as_pointer(dt->nativetype, e->right->token);
                }
                else
                {
                    cpu_set_acc_as_pointer(dt->nativetype, e->right->token);
                }
            }
            break;
        case KEY_SIZEOF:
            if(!e->right || e->right->tok != TOK_SYMBOL) error_expr(e, "function/variable expected inside addressof command.");
            ref_var = func_find_var(func, e->right->token);
            if(!ref_var) ref_var = var_find_global(e->right->token);
            if(ref_var)
            {
                if(to_aux_reg)
                {
                    cpu_set_aux(dt->nativetype, var_calc_total_size(e, ref_var));
                }
                else
                {
                    cpu_set_acc(dt->nativetype, var_calc_total_size(e, ref_var));
                }
            }
            else
            {
                ref_func = func_find(e->right->token);
                if(!ref_func) error_expr(e, "function/variable not found: %s", e->right->token);
                datatype_calcsize(e, datatype_find("pointer"));
                if(to_aux_reg)
                {
                    cpu_set_aux(dt->nativetype, datatype_find("pointer")->size);
                }
                else
                {
                    cpu_set_acc(dt->nativetype, datatype_find("pointer")->size);
                }
            }
            break;
        case ACT_POINTER_SYMBOL:
            ref_var = func_find_var(func, e->token);
            if(!ref_var) ref_var = var_find_global(e->token);
            if(!ref_var) error_expr(e, "variable not found: %s", e->token);
            if(ref_var->is_global) cpu_load_global_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name);
            else cpu_load_local_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name, ref_var->local_offset);
            codegen_autoconvert(e, datatype_find("pointer"), var_calc_datatype(e, ref_var));
            for(int i = 1; i < e->value; i++)
            {
                cpu_load_pointer(datatype_find("pointer")->nativetype);
            }
            cpu_load_pointer(ref_var->datatype->nativetype);
            codegen_autoconvert(e, dt, ref_var->datatype);
            break;
        case ACT_CALL:
            ref_func = func_find(e->token);
            if(ref_func) 
            {
                codegen_comment("CALL FUNCTION %s", e->token);
                args_size = codegen_args(func, cmd, e->right, ref_func, ref_func->args);
                cpu_call_function(var_calc_datatype(e, ref_func->return_model)->nativetype, e->token);
                codegen_autoconvert(e, dt, var_calc_datatype(e, ref_func->return_model));
                codegen_comment("END CALL FUNCTION %s", e->token);
            }
            else
            {
                codegen_comment("CALL VARIABLE %s", e->token);
                ref_var = func_find_var(func, e->token);
                if(!ref_var) ref_var = var_find_global(e->token);
                if(!ref_var) error_expr(e, "function/variable not found: %s", e->token);
                args_size = codegen_args(func, cmd, e->right, NULL, NULL);
                if(ref_var->is_global) cpu_load_global_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name);
                else cpu_load_local_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name, ref_var->local_offset);
                codegen_autoconvert(e, datatype_find("pointer"), var_calc_datatype(e, ref_var));
                cpu_call_acc();
                codegen_comment("END CALL VARIABLE %s", e->token);
            }
            cpu_restore_stack(args_size);
            break;
        case TOK_SYMBOL:
            ref_var = func_find_var(func, e->token);
            if(!ref_var) ref_var = var_find_global(e->token);
            if(ref_var)
            {
                if(ref_var->is_global)
                {
                    if(to_aux_reg) cpu_load_aux_global_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name);
                    else cpu_load_global_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name);
                    codegen_autoconvert(e, dt, var_calc_datatype(e, ref_var));
                }
                else
                {
                    if(to_aux_reg) cpu_load_local_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name, ref_var->local_offset);
                    else cpu_load_local_var(var_calc_datatype(e, ref_var)->nativetype, ref_var->name, ref_var->local_offset);
                    codegen_autoconvert(e, dt, var_calc_datatype(e, ref_var));
                }
            }
            else
            {
                ref_func = func_find(e->token);
                if(!ref_func) error_expr(e, "function/variable/constant not found: %s", e->token);
                if(to_aux_reg) cpu_set_aux_as_pointer(dt->nativetype, e->token);
                else cpu_set_acc_as_pointer(dt->nativetype, e->token);
            }
            break;
        default: error_expr(e, "unsupported expression type: %i[%s]", e->tok, e->token);
    }
    if(push_acc)
    {
        cpu_xchg_acc_aux(dt->nativetype);
        cpu_pop_acc(dt->nativetype);
    }
}

void codegen_asm(func_t *func, command_t *cmd)
{
    if(cmd->expression->tok != TOK_STRING) error_expr(cmd->expression, "assembly command string expected.");
    codegen_comment("INJECTED ASSEMBLY COMMAND");
    out_line("  %s", cmd->expression->token);
}

void codegen_if(func_t *func, command_t *cmd)
{
    int lbl_end = new_label();
    codegen_comment("IF");
    codegen_expr(func, cmd, cmd->expression, datatype_find("int"), false);
    cpu_jump_if_false(datatype_find("int")->nativetype, lbl_end);
    codegen_comment("THEN");
    command_t *child = cmd->contents;
    while(child)
    {
        codegen_command(func, child);
        child = child->next;
    }
    if(cmd->alt_contents)
    {
        int lbl_else = lbl_end;
        lbl_end = new_label();
        child = cmd->alt_contents;
        codegen_comment("ELSE");
        cpu_jump(lbl_end);
        cpu_label(lbl_else);
        while(child)
        {
            codegen_command(func, child);
            child = child->next;
        }
    }
    cpu_label(lbl_end);
    codegen_comment("END IF");
}

void codegen_while(func_t *func, command_t *cmd)
{
    int lbl_start = new_label();
    int lbl_end = new_label();
    codegen_comment("WHILE");
    cpu_label(lbl_start);
    codegen_expr(func, cmd, cmd->expression, datatype_find("int"), false);
    cpu_jump_if_false(datatype_find("int")->nativetype, lbl_end);
    codegen_comment("DO WHILE");
    command_t *child = cmd->contents;
    while(child)
    {
        codegen_command(func, child);
        child = child->next;
    }
    cpu_jump(lbl_start);
    cpu_label(lbl_end);
    codegen_comment("END WHILE");
}

void codegen_until(func_t *func, command_t *cmd)
{
    int lbl_start = new_label();
    int lbl_end = new_label();
    codegen_comment("UNTIL");
    cpu_label(lbl_start);
    codegen_expr(func, cmd, cmd->expression, datatype_find("int"), false);
    cpu_jump_if_true(datatype_find("int")->nativetype, lbl_end);
    codegen_comment("DO UNTIL");
    command_t *child = cmd->contents;
    while(child)
    {
        codegen_command(func, child);
        child = child->next;
    }
    cpu_jump(lbl_start);
    cpu_label(lbl_end);
    codegen_comment("END UNTIL");
}

void codegen_command(func_t *func, command_t *cmd)
{
    switch(cmd->cmd)
    {
        case CMD_NONE:
        case CMD_ELSE:
            break;
        case CMD_IF:
            codegen_if(func, cmd);
            break;
        case CMD_WHILE:
            codegen_while(func, cmd);
            break;
        case CMD_UNTIL:
            codegen_until(func, cmd);
            break;
        case CMD_ASM:
            codegen_asm(func, cmd);
            break;
        case CMD_RETURN:
            if(cmd->expression)
            {
                codegen_comment("RETURN EXPRESSION");
                codegen_expr(func, cmd, cmd->expression, var_calc_datatype(cmd->expression, func->return_model), false);
                codegen_comment("END RETURN EXPRESSION");
            }
            cpu_return_from_function();
            break;
        case CMD_EXPRESSION:
            codegen_comment("EXPRESSION COMMAND");
            codegen_expr(func, cmd, cmd->expression, datatype_find("int"), false);
            codegen_comment("END EXPRESSION COMMAND");
            break;
        default: error("unsupported command type: %i", cmd->cmd);
    }
}

void codegen_function(func_t *func)
{
    var_t *v = func->args;
    if(func->is_external) return;
    int32_t offset = cpu_function_args_offset();
    while(v)
    {
        v->local_offset = offset;
        offset += var_calc_total_size(v->error_reference, v);
        v = v->next;
    }
    int32_t var_size = 0;
    offset = cpu_function_vars_offset();
    v = func->vars;
    while(v)
    {
        offset -= var_calc_total_size(v->error_reference, v);
        v->local_offset = offset;
        var_size += var_calc_total_size(v->error_reference, v);
        v = v->next;
    }
    codegen_comment("FUNCTION: %s", func->name);
    cpu_function_start(func->name, var_size);

    command_t *cmd = func->contents;
    while(cmd)
    {
        codegen_command(func, cmd);
        cmd = cmd->next;
    }

    cpu_function_end(func->name, var_size);
    codegen_comment("END FUNCTION: %s", func->name);
}

void codegen()
{
    codegen_global_vars();
    func_t *func = func_get_list();
    while(func)
    {
        if(!func->is_global_context) codegen_function(func);
        func = func->next;
    }
}