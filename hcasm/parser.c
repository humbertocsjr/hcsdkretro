#include "asm.h"

char *_current_label = NULL;

expr_t *parse_expr();

bool is_prefix(expr_t *e)
{
    opcode_t *o = _prefix;
    while (o->mnemonic)
    {
        if (is_keyword(o->mnemonic, e->text))
            return true;
        o++;
    }
    return false;
}

bool is_mnemonic(expr_t *e)
{
    opcode_t *o = _opcode;
    while (o->mnemonic)
    {
        if (is_keyword(o->mnemonic, e->text))
            return true;
        o++;
    }
    return false;
}
opcode_t *parse_prefix(expr_t *e)
{
    opcode_t *o = _prefix;
    while (o->mnemonic)
    {
        if (is_keyword(o->mnemonic, e->text))
            return o;
        o++;
    }
    error_expr(e, "prefix unknown: %s", e->text);
    return NULL;
}

opcode_t *parse_mnemonic(expr_t *e)
{
    opcode_t *o = _opcode;
    while (o->mnemonic)
    {
        if (is_keyword(o->mnemonic, e->text))
            return o;
        o++;
    }
    error_expr(e, "mnemonic unknown: %s", e->text);
    return NULL;
}

expr_t *parse_expr0()
{
    expr_t *e = NULL;
    if (curr_is(TOK_SUB))
    {
        e = clone_expr(curr());
        e->left = clone_expr(curr());
        e->left->token = TOK_VALUE;
        e->left->value = 0;
        scan();
        e->right = parse_expr0();
    }
    else if (curr_is(TOK_VALUE) || curr_is(TOK_REGISTER) || curr_is(TOK_SYMBOL) || curr_is(TOK_CURRENT_POS))
    {
        e = clone_expr(curr());
        scan();
    }
    else if (curr_is(TOK_SUB_LABEL))
    {
        e = clone_expr(curr());
        e->token = TOK_SYMBOL;
        scan();
    }
    else if (curr_is(TOK_SUB))
    {
        e = clone_expr(curr());
        e->left = clone_expr(curr());
        e->left->token = TOK_VALUE;
        e->left->value = 0;
        scan();
        e->right = parse_expr0();
    }
    else if (curr_is(TOK_LOBYTE) || curr_is(TOK_HIBYTE))
    {
        e = clone_expr(curr());
        scan();
        e->right = parse_expr();
    }
    else if (curr_is(TOK_INDEX_OPEN))
    {
        e = clone_expr(curr());
        scan();
        e->right = parse_expr();
        if (!curr_is(TOK_INDEX_CLOSE))
            error_expr(curr(), "']' expected [Token: '%s'(#%i)]", curr()->text, curr()->token);
        scan();
    }
    else if (curr_is(TOK_PARAMS_OPEN))
    {
        scan();
        e = parse_expr();
        if (!curr_is(TOK_PARAMS_CLOSE))
            error_expr(curr(), "')' expected [Token: '%s'(#%i)]", curr()->text, curr()->token);
        scan();
    }
    else
        error_expr(curr(), "expression expected [Token: '%s'(#%i)]", curr()->text, curr()->token);
    return e;
}

expr_t *parse_expr1()
{
    return parse_expr0();
}

expr_t *parse_expr2()
{
    expr_t *e = parse_expr1();
    while (curr_is(TOK_MUL) || curr_is(TOK_DIV) || curr_is(TOK_MOD))
    {
        expr_t *op = clone_expr(curr());
        scan();
        op->left = e;
        op->right = parse_expr1();
        e = op;
    }
    return e;
}

expr_t *parse_expr3()
{
    expr_t *e = parse_expr2();
    if (e->token == TOK_REGISTER)
    {
        if (curr_is(TOK_ADD) || curr_is(TOK_SUB))
        {
            expr_t *op = clone_expr(curr());
            scan();
            op->left = e;
            op->right = parse_expr3();
            e = op;
        }
    }
    else
        while (curr_is(TOK_ADD) || curr_is(TOK_SUB))
        {
            expr_t *op = clone_expr(curr());
            scan();
            op->left = e;
            op->right = parse_expr2();
            e = op;
        }
    while (curr_is(TOK_COLON))
    {
        expr_t *op = clone_expr(curr());
        scan();
        op->left = e;
        op->right = parse_expr();
        e = op;
    }
    return e;
}

expr_t *parse_expr()
{
    return optimize(parse_expr3());
}

void parse_line()
{
    expr_t *mnemonic = NULL;
    int argc = 0;
    expr_t *argv[ARGV_MAX];
    while (is_prefix(curr()))
    {
        opcode_t *op = parse_prefix(curr());
        mnemonic = clone_expr(curr());
        scan();
        op->emit(mnemonic, op, 0, argv);
    }
    if (is_mnemonic(curr()))
    {
        opcode_t *op = parse_mnemonic(curr());
        mnemonic = clone_expr(curr());
        scan();
        while (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
        {
            if (curr_is_keyword("byte"))
            {
                mnemonic->force_byte = true;
                scan();
            }
            else if (curr_is_keyword("word"))
            {
                mnemonic->force_word = true;
                scan();
            }
            else if (curr_is_keyword("dword"))
            {
                mnemonic->force_dword = true;
                scan();
            }
            else if (curr_is_keyword("qword"))
            {
                mnemonic->force_qword = true;
                scan();
            }
            else if (curr_is_keyword("short"))
            {
                mnemonic->force_short = true;
                scan();
            }
            else if (curr_is_keyword("near"))
            {
                mnemonic->force_near = true;
                scan();
            }
            else if (curr_is_keyword("far"))
            {
                mnemonic->force_far = true;
                scan();
            }
            if (curr_is(TOK_HASH))
            {
                scan();
                argv[argc] = parse_expr();
                argv[argc]->immediate = true;
                argc++;
            }
            else
            {
                argv[argc++] = parse_expr();
            }
            if (!curr_is(TOK_COMMA))
                break;
            scan();
        }
        op->emit(mnemonic, op, argc, argv);
        for (int i = 0; i < argc; i++)
        {
            free_expr(argv[i]);
        }
    }
    else if (curr_is(TOK_SYMBOL) || curr_is(TOK_SUB_LABEL))
    {
        if (curr_is_keyword("db"))
        {
            scan();
            while (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
            {
                switch (curr()->token)
                {
                case TOK_VALUE:
                    out(REC_DATA, 0, 0, &curr()->value, 1);
                    scan();
                    break;
                case TOK_STRING:
                    out(REC_DATA, 0, 0, curr()->text, strlen(curr()->text));
                    scan();
                    break;
                default:;
                    expr_t *arg = parse_expr();
                    generate(arg, 0, false);
                    out(REC_EXPR_POP_INT8_EMIT, 0, 0, 0, 0);
                    free_expr(arg);
                    break;
                }
                if (!curr_is(TOK_COMMA))
                    break;
                scan();
            }
        }
        else if (curr_is_keyword("dw"))
        {
            scan();
            while (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
            {
                switch (curr()->token)
                {
                case TOK_VALUE:
                    out(REC_DATA, 0, 0, &curr()->value, 2);
                    scan();
                    break;
                case TOK_STRING:
                    out(REC_DATA, 0, 0, curr()->text, strlen(curr()->text));
                    scan();
                    break;
                default:;
                    expr_t *arg = parse_expr();
                    out(generate(arg, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
                    free_expr(arg);
                    break;
                }
                if (!curr_is(TOK_COMMA))
                    break;
                scan();
            }
        }
        else if (curr_is_keyword("global"))
        {
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(curr(), "name expected.");
            out(REC_CONST_AS_GLOBAL_LABEL, 0, 0, curr()->text, strlen(curr()->text));
            scan();
        }
        else if (curr_is_keyword("extern"))
        {
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(curr(), "name expected.");
            scan();
        }
        else if (curr_is_keyword("section"))
        {
            scan();
            if (curr_is_keyword("text"))
            {
                out(REC_SECTION_TEXT, 0, 0, 0, 0);
            }
            else if (curr_is_keyword("data"))
            {
                out(REC_SECTION_DATA, 0, 0, 0, 0);
            }
            else if (curr_is_keyword("bss"))
            {
                out(REC_SECTION_BSS, 0, 0, 0, 0);
            }
            else
                error_expr(curr(), "section name expected.");
            scan();
        }
        else if (curr_is_keyword("resb") || curr_is_keyword("rb") || curr_is_keyword("ds"))
        {
            scan();
            argv[0] = parse_expr();
            if (argv[0]->token != TOK_VALUE)
                error_expr(argv[0], "constant expression expected.");
            out(REC_DATA_RESERVE, argv[0]->value, 0, 0, 0);
            free_expr(argv[0]);
        }
        else if (curr_is_keyword("resw") || curr_is_keyword("rw"))
        {
            scan();
            argv[0] = parse_expr();
            if (argv[0]->token != TOK_VALUE)
                error_expr(argv[0], "constant expression expected.");
            out(REC_DATA_RESERVE, argv[0]->value * 2, 0, 0, 0);
            free_expr(argv[0]);
        }
        else if (curr_is_keyword("times"))
        {
            scan();
            argv[0] = parse_expr();
            generate(argv[0], 0, false);
            out(REC_EXPR_POP_REPEAT_TIMES, 0, 0, 0, 0);
            parse_line();
            out(REC_EXPR_REPEAT_TIMES_END, 0, 0, 0, 0);
            free_expr(argv[0]);
        }
        else
        {
            expr_t *name = clone_expr(curr());
            if (!curr_is(TOK_SUB_LABEL))
            {
                if (_current_label)
                    free(_current_label);
                _current_label = malloc(strlen(curr()->text) + 1);
                strcpy(_current_label, curr()->text);
            }
            scan();
            if (curr_is(TOK_COLON))
            {
                scan();
            }
            if (curr_is_keyword("equ"))
            {
                scan();
                expr_t *e = parse_expr();
                if (e->token != TOK_VALUE)
                    error_expr(e, "invalid constant expression");
                consts_set(name->text, e->value);
                out(REC_CONST_CUSTOM, e->value, 0, name->text, strlen(name->text));
                free_expr(name);
            }
            else
            {
                out(REC_CONST_LABEL, 0, 0, name->text, strlen(name->text));
                parse_line();
                free_expr(name);
                return;
            }
        }
    }
    free_expr(mnemonic);
    if (curr_is(TOK_SYMBOL))
        error_expr(curr(), "mnemonic expected. [Found: '%s'(#%i)]", curr()->text, curr()->token);
    if (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
        error_expr(curr(), "new line expected. [Found: '%s'(#%i)]", curr()->text, curr()->token);
}

void parse(char *filename)
{
    source_open(filename);
    scan();
    scan();
    out(REC_FILENAME, 0, 0, filename, strlen(filename));
    out(_cpu, 0, 0, 0, 0);
    out(REC_SECTION_TEXT, 0, 0, 0, 0);
    while (!curr_is(TOK_EOF))
    {
        while (curr_is(TOK_NEWLINE))
            scan();
        out(REC_POSITION, curr()->line, curr()->column, 0, 0);
        parse_line();
        while (curr_is(TOK_NEWLINE))
            scan();
    }
    out(REC_END_OF_FILE, 0, 0, filename, strlen(filename));
    source_close();
}
