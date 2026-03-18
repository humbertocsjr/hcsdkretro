#include "retrolang.h"

void parser_block(source_t *src, command_t *cmd, func_t *func, bool exit_on_else, bool to_alt_contents);

command_t *parser_variable_declaration(source_t *src, command_t *cmd, func_t *func)
{
    char name[TOKEN_SIZE];
    match_token(token_is(src, KEY_VAR), token_curr(src), "'var' expected.");
    token_scan(src);
    while(!token_is(src, TOK_NEWLINE))
    {
        match_token(token_is(src, TOK_SYMBOL), token_curr(src), "name expected.");
        strcpy(name, token_curr(src)->token);
        token_scan(src);
        match_token(token_is(src, KEY_AS), token_curr(src), "'as' expected.");
        token_scan(src);
        uint16_t array_size = 0;
        uint16_t pointer_level = 0;
        bool is_array = false;
        while(token_is(src, TOK_POINTER))
        {
            pointer_level++;
            token_scan(src);
        }
        datatype_t *dt = datatype_find(token_curr(src)->token);
        match_token(dt, token_curr(src), "datatype unknown: %s", token_curr(src)->token);
        token_scan(src);
        if(token_is(src, TOK_INDEX_OPEN))
        {
            is_array = true;
            token_scan(src);
            array_size = parse_const_expr(src, cmd, func);
            match_token(token_is(src, TOK_INDEX_CLOSE), token_curr(src), "']' expected.");
            token_scan(src);
            match_token(array_size > 0, token_curr(src), "invalid array size.");
        }
        match_token(!func_find_var(func, name), token_curr(src), "variable already declared: %s", name);
        func_add_var(func, name, dt, is_array, array_size, pointer_level > 0, pointer_level)->error_reference = convert_expr(token_curr(src));
        if(!token_is(src, TOK_COMMA)) break;
        token_scan(src);
    }
    return cmd;
}

command_t *parser_if(source_t *src, command_t *cmd, func_t *func)
{
    match_token(token_is(src, KEY_IF), token_curr(src), "'if' expected.");
    token_scan(src);
    cmd->cmd = CMD_IF;
    cmd->expression = parse_expr(src, cmd, func);
    parser_block(src, cmd, func, true, false);
    if(token_is(src, KEY_ELSE))
    {
        token_scan(src);
        parser_block(src, cmd, func, true, true);
    }
    return cmd;
}

command_t *parser_while(source_t *src, command_t *cmd, func_t *func)
{
    match_token(token_is(src, KEY_WHILE), token_curr(src), "'while' expected.");
    token_scan(src);
    cmd->cmd = CMD_WHILE;
    cmd->expression = parse_expr(src, cmd, func);
    parser_block(src, cmd, func, true, false);
    return cmd;
}

command_t *parser_until(source_t *src, command_t *cmd, func_t *func)
{
    match_token(token_is(src, KEY_UNTIL), token_curr(src), "'until' expected.");
    token_scan(src);
    cmd->cmd = CMD_UNTIL;
    cmd->expression = parse_expr(src, cmd, func);
    parser_block(src, cmd, func, true, false);
    return cmd;
}

command_t *parser_asm(source_t *src, command_t *cmd)
{
    match_token(token_is(src, KEY_ASM), token_curr(src), "'asm' expected.");
    token_scan(src);
    if(!strcmp(token_curr(src)->token, cpu_ext()))
    {
        cmd->cmd = CMD_ASM;
        token_scan(src);
        match_token(token_is(src, TOK_STRING), token_curr(src), "assembly command string expected.");
        cmd->expression = parse_expr(src, cmd, func_global());
    }
    else 
    {
        token_scan(src);
        token_scan(src);
    }
    return cmd;
}

command_t *parser_func_command(source_t *src, func_t *func)
{
    token_t *curr = token_curr(src);
    command_t *cmd = malloc(sizeof(command_t));
    if(!cmd) error_token(curr, "Command memory overflow.");
    memset(cmd, 0, sizeof(command_t));
    cmd->cmd = CMD_NONE;
    while(token_is(src, TOK_NEWLINE))
    {
        token_scan(src);
    }
    cmd->error_reference = convert_expr(token_curr(src));
    switch(curr->tok)
    {
        case KEY_VAR:
            parser_variable_declaration(src, cmd, func);
            break;
        case KEY_IF:
            parser_if(src, cmd, func);
            break;
        case KEY_WHILE:
            parser_while(src, cmd, func);
            break;
        case KEY_UNTIL:
            parser_until(src, cmd, func);
            break;
        case KEY_ASM:
            parser_asm(src, cmd);
            break;
        case KEY_RETURN:
            token_scan(src);
            if(!token_is(src, TOK_NEWLINE) && !token_is(src, TOK_EOF))
            {
                cmd->expression = parse_expr(src, cmd, func);
            }
            cmd->cmd = CMD_RETURN;
            break;
        default:
            cmd->expression = parse_expr(src, cmd, func);
            cmd->cmd = CMD_EXPRESSION;
            if(token_is(src, KEY_IF))
            {
                token_scan(src);
                command_t *cmd_if = malloc(sizeof(command_t));
                if(!cmd_if) error_token(curr, "Command memory overflow.");
                memset(cmd_if, 0, sizeof(command_t));
                cmd_if->cmd = CMD_IF;
                cmd_if->contents = cmd;
                cmd_if->expression = parse_expr(src, cmd, func);
                cmd = cmd_if;
            }
            break;
    }
    if(token_is(src, TOK_NEWLINE))
    {
        token_scan(src);
    }
    else match_token(token_is(src, TOK_EOF), token_curr(src), "new line expected.");
    return cmd;
}

void parser_block(source_t *src, command_t *cmd, func_t *func, bool exit_on_else, bool to_alt_contents)
{
    command_t *last = cmd ? (to_alt_contents ? cmd->alt_contents : cmd->contents) : func->contents;
    while(last && last->next)
    {
        last = last->next;
    }
    if(token_is(src, TOK_NEWLINE))
    {
        token_scan(src);
    }
    else match_token(token_is(src, TOK_EOF), token_curr(src), "new line expected.");
    while(!token_is(src, TOK_EOF) && !token_is(src, KEY_END) && (exit_on_else ? !token_is(src, KEY_ELSE) : true))
    {
        if(last)
        {
            last->next = parser_func_command(src, func);
            last = last->next;
        }
        else
        {
            last = parser_func_command(src, func);
            if(cmd && !to_alt_contents) cmd->contents = last;
            else if(cmd && to_alt_contents) cmd->alt_contents = last;
            else func->contents = last;
        }
    }
    if(token_is(src, TOK_EOF)) error_token(token_curr(src), "'end' expected.");
    if(token_is(src, KEY_END)) token_scan(src);
}

void parser_func_declaration(source_t *src)
{
    datatype_t *dt = datatype_find("int");
    char name[TOKEN_SIZE];
    char arg_name[TOKEN_SIZE];
    int arg_pointer_level;
    datatype_t *arg_dt;
    match_token(token_is(src, KEY_DEF), token_curr(src), "'def' expected.");
    token_scan(src);
    match_token(token_is(src, TOK_SYMBOL), token_curr(src), "function name expected.");
    strcpy(name, token_curr(src)->token);
    token_scan(src);
    func_t *func = func_add(name);
    func->error_reference = convert_expr(token_curr(src));
    match_token(token_is(src, TOK_PARAMS_OPEN), token_curr(src), "'(' expected.");
    token_scan(src);
    while(!token_is(src, TOK_PARAMS_CLOSE) && !token_is(src, TOK_EOF))
    {
        match_token(token_is(src, TOK_SYMBOL), token_curr(src), "argument name expected.");
        strcpy(arg_name, token_curr(src)->token);
        token_scan(src);
        match_token(token_is(src, KEY_AS), token_curr(src), "'as' expected.");
        token_scan(src);
        arg_pointer_level = 0;
        while(token_is(src, TOK_POINTER))
        {
            arg_pointer_level++;
            token_scan(src);
        }
        arg_dt = datatype_find(token_curr(src)->token);
        match_token(arg_dt, token_curr(src), "known datatype expected.");
        token_scan(src);
        func_add_arg(func, arg_name, arg_dt, arg_pointer_level > 0, arg_pointer_level);
        if(!token_is(src, TOK_COMMA)) break;
        token_scan(src);
    }
    match_token(token_is(src, TOK_PARAMS_CLOSE), token_curr(src), "')' expected.");
    token_scan(src);
    if(token_is(src, KEY_AS))
    {
        token_scan(src);
        arg_pointer_level = 0;
        while(token_is(src, TOK_POINTER))
        {
            arg_pointer_level++;
            token_scan(src);
        }
        dt = datatype_find(token_curr(src)->token);
        match_token(dt, token_curr(src), "known datatype expected.");
        func->return_model = func_add_var(func, "__RETURN__", dt, false, 0, arg_pointer_level > 0, arg_pointer_level);
        token_scan(src);
    }
    else func->return_model = func_add_var(func, "__RETURN__", dt, false, 0, false, 0);
    func->return_model->error_reference = convert_expr(token_curr(src));
    func->vars = NULL;
    parser_block(src, NULL, func, false, false);
}

void parser_declaration(source_t *src, command_t *cmd)
{
    datatype_t *dt = datatype_find("int");
    int pointer_level;
    char name[TOKEN_SIZE];
    bool is_array;
    int array_size;
    func_t *func;
    char arg_name[TOKEN_SIZE];
    int arg_pointer_level;
    datatype_t *arg_dt;
    match_token(token_is(src, KEY_DECL), token_curr(src), "'decl' expected.");
    token_scan(src);
    match_token(token_is(src, TOK_SYMBOL), token_curr(src), "name expected.");
    strcpy(name, token_curr(src)->token);
    token_scan(src);
    if(token_is(src, KEY_AS))
    {
        token_scan(src);
        pointer_level = 0;
        while(token_is(src, TOK_POINTER))
        {
            pointer_level++;
            token_scan(src);
        }
        dt = datatype_find(token_curr(src)->token);
        if(!dt) error_token(token_curr(src), "known datatype expected.");
        token_scan(src);
        is_array = false;
        if(token_is(src, TOK_INDEX_OPEN))
        {
            is_array = true;
            token_scan(src);
            array_size = parse_const_expr(src, cmd, func_global());
            match_token(token_is(src, TOK_INDEX_CLOSE), token_curr(src), "']' expected.");
            token_scan(src);
        }
        codegen_comment("DECLARED EXTERNAL VARIABLE: %s", name);
        var_add_global(name, dt, is_array, array_size, pointer_level > 0, pointer_level);
    }
    else if(token_is(src, TOK_PARAMS_OPEN))
    {
        codegen_comment("DECLARED EXTERNAL FUNCTION: %s", name);
        func = func_add(name);
        func->is_external = true;
        token_scan(src);
        while(!token_is(src, TOK_PARAMS_CLOSE) && !token_is(src, TOK_EOF))
        {
            match_token(token_is(src, TOK_SYMBOL), token_curr(src), "argument name expected.");
            strcpy(arg_name, token_curr(src)->token);
            token_scan(src);
            match_token(token_is(src, KEY_AS), token_curr(src), "'as' expected.");
            token_scan(src);
            arg_pointer_level = 0;
            while(token_is(src, TOK_POINTER))
            {
                arg_pointer_level++;
                token_scan(src);
            }
            arg_dt = datatype_find(token_curr(src)->token);
            match_token(arg_dt, token_curr(src), "known datatype expected.");
            token_scan(src);
            func_add_arg(func, arg_name, arg_dt, arg_pointer_level > 0, arg_pointer_level);
            if(!token_is(src, TOK_COMMA)) break;
            token_scan(src);
        }
        match_token(token_is(src, TOK_PARAMS_CLOSE), token_curr(src), "')' expected.");
        token_scan(src);
        if(token_is(src, KEY_AS))
        {
            token_scan(src);
            arg_pointer_level = 0;
            while(token_is(src, TOK_POINTER))
            {
                arg_pointer_level++;
                token_scan(src);
            }
            dt = datatype_find(token_curr(src)->token);
            match_token(dt, token_curr(src), "known datatype expected.");
            token_scan(src);
            func->return_model = func_add_var(func, "__RETURN__", dt, false, 0, arg_pointer_level > 0, arg_pointer_level);
        }
        else func->return_model = func_add_var(func, "__RETURN__", dt, false, 0, false, 0);
        
        func->return_model->error_reference = convert_expr(token_curr(src));
        func->vars = NULL;
    }
    else
    {
        error_token(token_curr(src), "invalid external declaration.");
    }
}

command_t *parser_root_command(source_t *src)
{
    token_t *curr = token_curr(src);
    command_t *cmd = malloc(sizeof(command_t));
    if(!cmd) error_token(curr, "Command memory overflow.");
    memset(cmd, 0, sizeof(command_t));
    cmd->cmd = CMD_NONE;
    while(token_is(src, TOK_NEWLINE))
    {
        token_scan(src);
    }
    switch(curr->tok)
    {
        case TOK_EOF:
            break;
        case KEY_VAR:
            parser_variable_declaration(src, cmd, func_global());
            break;
        case KEY_DEF:
            parser_func_declaration(src);
            break;
        case KEY_ASM:
            parser_asm(src, cmd);
            break;
        case KEY_DECL:
            parser_declaration(src, cmd);
            break;
        default:
            error_token(curr, "invalid root command: %s", curr->token);
            break;
    }
    if(token_is(src, TOK_NEWLINE))
    {
        token_scan(src);
    }
    else match_token(token_is(src, TOK_EOF), token_curr(src), "new line expected.");
    return cmd;
}

void parser_process_file(char *filename)
{
    func_t *context = func_global();
    command_t *last = context->contents;
    while(last && last->next)
    {
        last = last->next;
    }
    source_t *src = source_open(filename);
    while(!token_is(src, TOK_EOF))
    {
        if(last)
        {
            last->next = parser_root_command(src);
            last = last->next;
        }
        else
        {
            last = parser_root_command(src);
            context->contents = last;
        }
    }
    source_close(src);
}