#include "bcomp.h"

static int current_label = 0;
static int func_nlocals = 0;
static int func_nparams = 0;
static int func_has_return = 0;
static int break_label = -1;

static void expect(token_t t)
{
    if (tok != t)
        error("expected token %i, got %i ('%s')", t, tok, tok_text);
    next();
}

static void convert_rvalue(int *kind)
{
    if (*kind)
    {
        gen_deref();
        *kind = VAL_RVALUE;
    }
}

// --== Declarations ==--

static void parse_params(void)
{
    expect(TOK_LPAREN);
    func_nparams = 0;
    if (tok != TOK_RPAREN)
    {
        while (1)
        {
            if (tok != TOK_IDENT)
                error("parameter name expected");
            install(tok_text, SYM_PARAM, 0);
            func_nparams++;
            next();
            if (tok != TOK_COMMA)
                break;
            next();
        }
    }
    expect(TOK_RPAREN);
}

static void parse_declaration(void)
{
    int is_auto = (tok == TOK_AUTO);
    int is_extrn = (tok == TOK_EXTRN);

    if (!is_auto && !is_extrn)
        return;
    next();

    do
    {
        if (tok != TOK_IDENT)
            error("variable name expected");
        const char *name = strdup(tok_text);
        next();

        int arr_size = 0;
        if (tok == TOK_LBRACKET)
        {
            next();
            arr_size = tok_value;
            if (tok != TOK_NUMBER)
                error("array size expected");
            next();
            expect(TOK_RBRACKET);
        }

        if (is_extrn)
        {
            install(name, SYM_EXTERN, arr_size);
            gen_extern(name);
        }
        else if (is_auto)
        {
            symbol_t *sym = install(name, SYM_LOCAL, arr_size);
            if (arr_size > 0)
                sym->offset = func_nlocals++;
            else
                sym->offset = func_nlocals++;
        }

        if (tok == TOK_COMMA)
            next();
        else
            break;
    } while (1);

    expect(TOK_SEMICOLON);
}

// --== Expression parsing ==--

static int primary(void);
static int postfix(void);
static int unary(void);
static int term(void);
static int additive(void);
static int shift(void);
static int relational(void);
static int equality(void);
static int bitwise_and(void);
static int bitwise_xor(void);
static int bitwise_or(void);
static int logical_and(void);
static int logical_or(void);
static int parse_args_reverse(void);

static int primary(void)
{
    int kind = VAL_RVALUE;

    if (tok == TOK_NUMBER)
    {
        gen_load_imm(tok_value);
        next();
    }
    else if (tok == TOK_CHAR)
    {
        gen_load_imm(tok_value);
        next();
    }
    else if (tok == TOK_STRING)
    {
        int l = gen_label();
        gen_data();
        gen_label_int(l);
        gen_bytes(tok_text);
        gen_word(0);
        gen_text();
        gen_load_imm(l);
        next();
        // Strings evaluate to their address
        kind = VAL_LVALUE; // address is in HL, can dereference
    }
    else if (tok == TOK_IDENT)
    {
        symbol_t *sym = lookup(tok_text);
        if (!sym)
        {
            // Implicit declaration as external
            sym = install(tok_text, SYM_EXTERN, 0);
            gen_extern(tok_text);
        }
        next();

        if (sym->kind == SYM_LOCAL)
        {
            gen_load_local(sym->offset);
            kind = VAL_LVALUE;
        }
        else if (sym->kind == SYM_PARAM)
        {
            gen_load_param(sym->offset);
            kind = VAL_LVALUE;
        }
        else
        {
            gen_load_addr(sym->name);
            kind = VAL_LVALUE;
        }
    }
    else if (tok == TOK_LPAREN)
    {
        next();
        kind = expression();
        expect(TOK_RPAREN);
    }
    else
    {
        error("expression expected, got '%s'", tok_text);
    }

    return kind;
}

static int postfix(void)
{
    int kind = primary();

    while (1)
    {
        if (tok == TOK_LBRACKET)
        {
            // arr[index] -> *(arr + index * 2)
            // arr is lvalue (address in HL)
            convert_rvalue(&kind);
            gen_push_prim(); // save address

            next();
            int idx_kind = expression();
            convert_rvalue(&idx_kind);
            expect(TOK_RBRACKET);

            // HL = index, DE = address (from stack)
            gen_pop_sec();

            // index * 2 (word size): base + index*2
            gen_double(); // HL = index * 2
            gen_add();    // HL = base + index*2
            // Now HL = address of arr[index]
            kind = VAL_LVALUE;
        }
        else if (tok == TOK_LPAREN)
        {
            // Function call
            // Primary parsed a name or function pointer
            // For function calls from a name, we need the function name
            // But primary() parsed it and loaded its address into HL
            // For a simple call like func(), we need func as a label

            // Hmm, this doesn't work. We need the function name for the call.
            // Let me save the function name before parsing arguments.
            // Actually, for a simple identifier, we can extract the name.

            // The primary() loaded the address. But for the call, we need the name.
            // This is a design issue. Let me think...

            // Actually, for a direct call like `func(args)`, the parser should
            // handle this BEFORE primary() loads anything. The issue is that
            // my parser already consumed the identifier.

            // Let me fix this: I'll track the last identifier name in the parser.
            // Or better: for function calls, I'll handle the name separately.

            // Actually, looking at this more carefully, for `func()`:
            // postfix -> primary (parses "func" as identifier, loads address)
            // Then postfix sees TOK_LPAREN and generates a call
            // But we need the NAME for the call instruction.

            // The problem is that primary() consumed the name and we lost it.
            // I need to save it. Let me use a global variable.

            error("indirect function calls not yet supported");
        }
        else if (tok == TOK_INC)
        {
            convert_rvalue(&kind);
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_add();
            gen_pop_sec(); // DE = address
            gen_store_to_addr();
            gen_push_prim(); // result is old value + 1
            next();
            kind = VAL_RVALUE;
        }
        else if (tok == TOK_DEC)
        {
            convert_rvalue(&kind);
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_sub();
            gen_pop_sec();
            gen_store_to_addr();
            gen_push_prim();
            next();
            kind = VAL_RVALUE;
        }
        else
        {
            break;
        }
    }

    return kind;
}

// For function call handling, we save the last parsed identifier
static char last_func_name[256];

// Redefined primary for function call support
static int primary_func(void)
{
    int kind = VAL_RVALUE;

    if (tok == TOK_NUMBER)
    {
        gen_load_imm(tok_value);
        next();
    }
    else if (tok == TOK_CHAR)
    {
        gen_load_imm(tok_value);
        next();
    }
    else if (tok == TOK_STRING)
    {
        int l = gen_label();
        gen_data();
        gen_label_int(l);
        gen_bytes(tok_text);
        gen_word(0);
        gen_text();
        gen_load_imm(l);
        next();
        kind = VAL_LVALUE;
    }
    else if (tok == TOK_IDENT)
    {
        strncpy(last_func_name, tok_text, 255);
        last_func_name[255] = '\0';
        symbol_t *sym = lookup(tok_text);
        if (!sym)
        {
            sym = install(tok_text, SYM_EXTERN, 0);
            gen_extern(tok_text);
        }
        next();

        if (sym->kind == SYM_LOCAL)
        {
            gen_local_addr(sym->offset);
            kind = VAL_LVALUE;
        }
        else if (sym->kind == SYM_PARAM)
        {
            gen_param_addr(sym->offset);
            kind = VAL_LVALUE;
        }
        else
        {
            gen_load_addr(sym->name);
            kind = VAL_LVALUE;
        }
    }
    else if (tok == TOK_LPAREN)
    {
        next();
        kind = expression();
        expect(TOK_RPAREN);
    }
    else
    {
        error("expression expected, got '%s'", tok_text);
    }

    return kind;
}

static int postfix_func(void)
{
    int kind = primary_func();
    char funcname[256];
    int has_func_name = 0;

    // Check if this was a simple function name
    // (last_func_name is set by primary_func for identifiers)
    if (kind == VAL_LVALUE && last_func_name[0])
    {
        symbol_t *sym = lookup(last_func_name);
        if (sym && (sym->kind == SYM_EXTERN || sym->kind == SYM_FUNCTION))
        {
            strcpy(funcname, last_func_name);
            has_func_name = 1;
        }
    }

    while (1)
    {
        if (tok == TOK_LBRACKET)
        {
            convert_rvalue(&kind);
            gen_push_prim();
            next();
            int idx_kind = expression();
            convert_rvalue(&idx_kind);
            expect(TOK_RBRACKET);
            gen_pop_sec();
            gen_double();
            gen_add();
            kind = VAL_LVALUE;
            has_func_name = 0;
        }
        else if (tok == TOK_LPAREN)
        {
            if (!has_func_name)
                error("function call requires function name");

            /* Inline peekb/pokeb/pokew/peekw */
            if (!strcmp(funcname, "peekb"))
            {
                next();
                int pk = assignment(); /* addr */
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline peekb");
                gen_peekb();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }
            if (!strcmp(funcname, "peekw"))
            {
                next();
                int pk = assignment();
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline peekw");
                gen_deref();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }
            if (!strcmp(funcname, "pokeb"))
            {
                next();
                int pk = assignment(); /* addr (left arg) */
                convert_rvalue(&pk);
                gen_push_prim(); /* save addr on stack */
                expect(TOK_COMMA);
                pk = assignment(); /* val (right arg) */
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline pokeb");
                gen_pop_sec(); /* DE = addr, HL = val */
                gen_pokeb();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }
            if (!strcmp(funcname, "pokew"))
            {
                next();
                int pk = assignment();
                convert_rvalue(&pk);
                gen_push_prim();
                expect(TOK_COMMA);
                pk = assignment();
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline pokew");
                gen_pop_sec();
                gen_store_to_addr();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }

            next();
            gen_comment("call %s", funcname);
            int nargs = 0;
            if (tok != TOK_RPAREN)
            {
                nargs = parse_args_reverse();
            }
            expect(TOK_RPAREN);

            gen_comment("call %s(%i args)", funcname, nargs);
            gen_call(funcname, nargs);
            kind = VAL_RVALUE;
            has_func_name = 0;
            last_func_name[0] = 0;
        }
        else if (tok == TOK_INC)
        {
            convert_rvalue(&kind);
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_add();
            gen_pop_sec();
            gen_store_to_addr();
            next();
            kind = VAL_RVALUE;
        }
        else if (tok == TOK_DEC)
        {
            convert_rvalue(&kind);
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_sub();
            gen_pop_sec();
            gen_store_to_addr();
            next();
            kind = VAL_RVALUE;
        }
        else
        {
            break;
        }
    }

    return kind;
}

static int unary(void)
{
    if (tok == TOK_STAR)
    {
        next();
        int kind = unary();
        // *expr: expr is the address, result is the value at that address
        // The address should already be in HL
        // Dereference: load value from HL
        // But we also want *expr to be an lvalue (for *expr = value)
        // After unary(), kind is LVALUE or RVALUE
        // If LVALUE: HL already has address. *expr -> address is in HL.
        // If RVALUE: HL has value. *value -> treat value as address.
        // In both cases, HL has the address. Return LVALUE.
        return VAL_LVALUE;
    }
    else if (tok == TOK_AMPERSAND)
    {
        next();
        int kind = unary();
        if (!kind)
            error("lvalue required as operand of &");
        // &lvalue: address is already in HL
        return VAL_RVALUE;
    }
    else if (tok == TOK_MINUS)
    {
        next();
        int kind = unary();
        convert_rvalue(&kind);
        gen_neg();
        return VAL_RVALUE;
    }
    else if (tok == TOK_TILDE)
    {
        next();
        int kind = unary();
        convert_rvalue(&kind);
        gen_not();
        return VAL_RVALUE;
    }
    else if (tok == TOK_BANG)
    {
        next();
        int kind = unary();
        convert_rvalue(&kind);
        gen_lnot();
        return VAL_RVALUE;
    }
    else if (tok == TOK_INC)
    {
        next();
        int kind = unary();
        if (!kind)
            error("lvalue required as operand of ++");
        gen_push_prim(); // save address
        gen_deref();     // HL = value
        gen_push_prim(); // save value for return
        gen_load_imm(1);
        gen_pop_sec();       // DE = 1
        gen_add();           // HL = value + 1
        gen_pop_sec();       // DE = address
        gen_store_to_addr(); // *address = value+1
        return VAL_RVALUE;
    }
    else if (tok == TOK_DEC)
    {
        next();
        int kind = unary();
        if (!kind)
            error("lvalue required as operand of --");
        gen_push_prim(); // save address
        gen_deref();     // HL = value
        gen_push_prim(); // save value for return
        gen_load_imm(1);
        gen_pop_sec();       // DE = 1
        gen_sub();           // HL = value - 1
        gen_pop_sec();       // DE = address
        gen_store_to_addr(); // *address = value-1
        return VAL_RVALUE;
    }
    else
    {
        return postfix_func();
    }
}

static int term(void)
{
    int kind = unary();

    while (tok == TOK_STAR || tok == TOK_SLASH || tok == TOK_PERCENT)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        int op = tok;
        next();
        int right_kind = unary();
        convert_rvalue(&right_kind);
        gen_pop_sec();

        switch (op)
        {
        case TOK_STAR:
            gen_mul();
            break;
        case TOK_SLASH:
            gen_div();
            break;
        case TOK_PERCENT:
            gen_mod();
            break;
        }
        kind = VAL_RVALUE;
    }
    return kind;
}

static int additive(void)
{
    int kind = term();

    while (tok == TOK_PLUS || tok == TOK_MINUS)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        int op = tok;
        next();
        int right_kind = term();
        convert_rvalue(&right_kind);
        gen_pop_sec();

        if (op == TOK_PLUS)
            gen_add();
        else
            gen_sub();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int shift(void)
{
    int kind = additive();

    while (tok == TOK_LSHIFT || tok == TOK_RSHIFT)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        int op = tok;
        next();
        int right_kind = additive();
        convert_rvalue(&right_kind);
        gen_pop_sec();

        if (op == TOK_LSHIFT)
            gen_shl();
        else
            gen_shr();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int relational(void)
{
    int kind = shift();

    while (tok == TOK_LT || tok == TOK_GT || tok == TOK_LE || tok == TOK_GE)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        int op = tok;
        next();
        int right_kind = shift();
        convert_rvalue(&right_kind);
        gen_pop_sec();

        switch (op)
        {
        case TOK_LT:
            gen_cmp_lt();
            break;
        case TOK_GT:
            gen_cmp_gt();
            break;
        case TOK_LE:
            gen_cmp_le();
            break;
        case TOK_GE:
            gen_cmp_ge();
            break;
        }
        kind = VAL_RVALUE;
    }
    return kind;
}

static int equality(void)
{
    int kind = relational();

    while (tok == TOK_EQ || tok == TOK_NE)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        int op = tok;
        next();
        int right_kind = relational();
        convert_rvalue(&right_kind);
        gen_pop_sec();

        if (op == TOK_EQ)
            gen_cmp_eq();
        else
            gen_cmp_ne();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int bitwise_and(void)
{
    int kind = equality();

    while (tok == TOK_AMPERSAND)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        next();
        int right_kind = equality();
        convert_rvalue(&right_kind);
        gen_pop_sec();
        gen_and();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int bitwise_xor(void)
{
    int kind = bitwise_and();

    while (tok == TOK_CARET)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        next();
        int right_kind = bitwise_and();
        convert_rvalue(&right_kind);
        gen_pop_sec();
        gen_xor();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int bitwise_or(void)
{
    int kind = bitwise_xor();

    while (tok == TOK_PIPE)
    {
        convert_rvalue(&kind);
        gen_push_prim();
        next();
        int right_kind = bitwise_xor();
        convert_rvalue(&right_kind);
        gen_pop_sec();
        gen_or();
        kind = VAL_RVALUE;
    }
    return kind;
}

static int logical_and(void)
{
    int kind = bitwise_or();

    if (tok == TOK_AND)
    {
        convert_rvalue(&kind);
        int l_false = gen_label();
        int l_done = gen_label();
        gen_comment("&& short-circuit");
        gen_jz(l_false);

        while (tok == TOK_AND)
        {
            next();
            kind = bitwise_or();
            convert_rvalue(&kind);
            gen_jz(l_false);
        }

        gen_load_imm(1);
        gen_jmp(l_done);
        gen_label_int(l_false);
        gen_load_imm(0);
        gen_label_int(l_done);
        return VAL_RVALUE;
    }
    return kind;
}

static int logical_or(void)
{
    int kind = logical_and();

    if (tok == TOK_OR)
    {
        int l_true = gen_label();
        int l_done = gen_label();
        convert_rvalue(&kind);
        gen_comment("|| short-circuit");
        gen_jnz(l_true);

        while (tok == TOK_OR)
        {
            next();
            kind = logical_and();
            convert_rvalue(&kind);
            gen_jnz(l_true);
        }

        gen_load_imm(0);
        gen_jmp(l_done);
        gen_label_int(l_true);
        gen_load_imm(1);
        gen_label_int(l_done);
        return VAL_RVALUE;
    }
    return kind;
}

int conditional(void)
{
    return logical_or();
}

int assignment(void)
{
    int kind = conditional();

    if (tok == TOK_ASSIGN)
    {
        if (!kind)
            error("lvalue required as left operand of assignment");
        gen_push_prim();
        next();
        int right_kind = assignment();
        gen_pop_sec();
        gen_store_to_addr();
        return VAL_RVALUE;
    }

    if (tok == TOK_ADD_ASSIGN || tok == TOK_SUB_ASSIGN ||
        tok == TOK_MUL_ASSIGN || tok == TOK_DIV_ASSIGN ||
        tok == TOK_MOD_ASSIGN || tok == TOK_AND_ASSIGN ||
        tok == TOK_OR_ASSIGN || tok == TOK_XOR_ASSIGN)
    {
        if (!kind)
            error("lvalue required as left operand of compound assignment");

        int op = tok;
        gen_push_prim(); // save address
        gen_deref();     // HL = current value
        gen_push_prim(); // save current value on stack
        next();
        int right_kind = assignment();
        gen_pop_sec(); // DE = current value, HL = right value

        switch (op)
        {
        case TOK_ADD_ASSIGN:
            gen_add();
            break;
        case TOK_SUB_ASSIGN:
            gen_sub();
            break;
        case TOK_MUL_ASSIGN:
            gen_mul();
            break;
        case TOK_DIV_ASSIGN:
            gen_div();
            break;
        case TOK_MOD_ASSIGN:
            gen_mod();
            break;
        case TOK_AND_ASSIGN:
            gen_and();
            break;
        case TOK_OR_ASSIGN:
            gen_or();
            break;
        case TOK_XOR_ASSIGN:
            gen_xor();
            break;
        }

        gen_pop_sec();       // DE = address
        gen_store_to_addr(); // *address = result
        return VAL_RVALUE;
    }

    return kind;
}

int expression(void)
{
    int kind = assignment();

    while (tok == TOK_COMMA)
    {
        next();
        kind = assignment();
    }
    return kind;
}

// --== Function call args (right-to-left push for C convention) ==--

static int parse_args_reverse(void)
{
    int k = assignment();
    int count = 1;
    if (tok == TOK_COMMA)
    {
        next();
        count += parse_args_reverse();
    }
    convert_rvalue(&k);
    gen_push_prim();
    return count;
}

// --== Statements ==--

static void parse_statement(void)
{
    if (tok == TOK_ASM)
    {
        next();
        expect(TOK_LPAREN);
        if (tok != TOK_STRING)
            error("string expected");
        gen_emit_raw(tok_text);
        next();
        expect(TOK_RPAREN);
        expect(TOK_SEMICOLON);
    }
    else if (tok == TOK_LBRACE)
    {
        compound_statement();
    }
    else if (tok == TOK_IF)
    {
        next();
        expect(TOK_LPAREN);
        int kind = expression();
        convert_rvalue(&kind);
        expect(TOK_RPAREN);

        int l_else = gen_label();
        int l_done = gen_label();

        gen_comment("if");
        gen_jz(l_else);
        parse_statement();

        if (tok == TOK_ELSE)
        {
            gen_jmp(l_done);
            gen_label_int(l_else);
            next();
            parse_statement();
            gen_label_int(l_done);
        }
        else
        {
            gen_label_int(l_else);
        }
    }
    else if (tok == TOK_WHILE)
    {
        int l_test = gen_label();
        int l_body = gen_label();
        int l_done = gen_label();
        int saved_break = break_label;
        break_label = l_done;

        next();
        gen_label_int(l_test);
        expect(TOK_LPAREN);
        int kind = expression();
        convert_rvalue(&kind);
        expect(TOK_RPAREN);

        gen_comment("while");
        gen_jz(l_done);
        parse_statement();
        gen_jmp(l_test);
        gen_label_int(l_done);

        break_label = saved_break;
    }
    else if (tok == TOK_FOR)
    {
        int l_test = gen_label();
        int l_body = gen_label();
        int l_done = gen_label();
        int saved_break = break_label;
        break_label = l_done;

        next();
        expect(TOK_LPAREN);

        // init
        if (tok != TOK_SEMICOLON)
        {
            int kind = expression();
        }
        expect(TOK_SEMICOLON);

        gen_label_int(l_test);
        // test
        if (tok != TOK_SEMICOLON)
        {
            int kind = expression();
            convert_rvalue(&kind);
            gen_comment("for test");
            gen_jz(l_done);
        }
        expect(TOK_SEMICOLON);

        // Parse increment, emit to temp buffer for later replay
        FILE *inc_fp = tmpfile();
        FILE *saved_out = outfile;
        outfile = inc_fp;

        if (tok != TOK_RPAREN)
        {
            gen_comment("for increment");
            int inc_kind = expression();
            // Don't need to convert - expression value is unused
        }
        expect(TOK_RPAREN);

        long inc_size = ftell(inc_fp);
        rewind(inc_fp);

        outfile = saved_out;

        parse_statement();

        // Emit deferred increment code
        if (inc_size > 0)
        {
            char inc_buf[4096];
            while (fgets(inc_buf, sizeof(inc_buf), inc_fp))
            {
                fprintf(outfile, "%s", inc_buf);
            }
        }
        fclose(inc_fp);

        gen_jmp(l_test);
        gen_label_int(l_done);

        break_label = saved_break;
    }
    else if (tok == TOK_DO)
    {
        int l_body = gen_label();
        int l_done = gen_label();
        int saved_break = break_label;
        break_label = l_done;

        next();
        gen_label_int(l_body);
        parse_statement();
        expect(TOK_WHILE);
        expect(TOK_LPAREN);
        int kind = expression();
        convert_rvalue(&kind);
        expect(TOK_RPAREN);
        expect(TOK_SEMICOLON);

        gen_comment("do-while test");
        gen_jnz(l_body);
        gen_label_int(l_done);

        break_label = saved_break;
    }
    else if (tok == TOK_RETURN)
    {
        next();
        if (tok != TOK_SEMICOLON)
        {
            int kind = expression();
            convert_rvalue(&kind);
        }
        expect(TOK_SEMICOLON);
        gen_comment("return");
        gen_return();
    }
    else if (tok == TOK_BREAK)
    {
        next();
        expect(TOK_SEMICOLON);
        if (break_label < 0)
            error("break outside loop");
        gen_jmp(break_label);
    }
    else if (tok == TOK_SEMICOLON)
    {
        next();
    }
    else if (tok == TOK_AUTO || tok == TOK_EXTRN)
    {
        parse_declaration();
    }
    else if (tok == TOK_IDENT || tok == TOK_NUMBER || tok == TOK_CHAR ||
             tok == TOK_STRING || tok == TOK_LPAREN || tok == TOK_STAR ||
             tok == TOK_AMPERSAND || tok == TOK_MINUS || tok == TOK_TILDE ||
             tok == TOK_BANG || tok == TOK_INC || tok == TOK_DEC)
    {
        int kind = expression();
        expect(TOK_SEMICOLON);
    }
    else
    {
        error("unexpected token: '%s'", tok_text);
    }
}

void compound_statement(void)
{
    expect(TOK_LBRACE);
    while (tok != TOK_RBRACE && tok != TOK_EOF)
    {
        parse_statement();
    }
    expect(TOK_RBRACE);
}

// --== Top-level ==--

static void function_definition(const char *name)
{
    gen_global(name);
    gen_text();

    func_nlocals = 0;
    func_nparams = 0;
    break_label = -1;

    parse_params();

    expect(TOK_LBRACE);

    // Parse all declarations first to count locals
    while (tok == TOK_AUTO || tok == TOK_EXTRN)
    {
        parse_declaration();
    }

    gen_comment("function %s prologue", name);
    gen_prologue(name, func_nlocals);

    // Parse remaining statements
    while (tok != TOK_RBRACE && tok != TOK_EOF)
    {
        parse_statement();
    }

    expect(TOK_RBRACE);
    gen_comment("function %s epilogue", name);
    gen_epilogue();
}

void compile_unit(void)
{
    gen_text();
    
    while (tok != TOK_EOF) {
        if (tok == TOK_AUTO || tok == TOK_EXTRN) {
            parse_declaration();
        } else if (tok == TOK_IDENT) {
            const char *name = strdup(tok_text);
            next();
            
            if (tok == TOK_LPAREN) {
                function_definition(name);
            } else if (tok == TOK_SEMICOLON || tok == TOK_COMMA || tok == TOK_LBRACKET) {
                symbol_t *sym = install(name, SYM_GLOBAL, 0);
                gen_data();
                gen_global(name);
                
                if (tok == TOK_LBRACKET) {
                    next();
                    int size = tok_value;
                    if (tok != TOK_NUMBER) error("array size expected");
                    next();
                    expect(TOK_RBRACKET);
                    sym->size = size;
                    gen_label_str(name);
                    gen_reserve(size * 2);
                } else {
                    gen_label_str(name);
                    gen_word(0);
                }
                gen_text();
                expect(TOK_SEMICOLON);
            } else {
                error("unexpected token after identifier: '%s'", tok_text);
            }
        } else {
            error("unexpected token at top level: '%s'", tok_text);
        }
    }
    
    gen_data_final();
}
