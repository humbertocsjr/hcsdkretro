#include "bcomp.h"

static int current_label = 0;
static int func_nlocals = 0;
static int func_nparams = 0;
static int func_has_return = 0;
static int func_has_return_at_end = 0;
static int break_label = -1;

// [English] Expects the current token to be of type t, then advances to the next token.
// Terminates with an error if the token does not match.
// [Portuguese] Espera que o token atual seja do tipo t, então avança para o próximo token.
// Encerra com erro se o token não corresponder.
static void expect(token_t t)
{
    if (tok != t)
        error("expected token %i, got %i ('%s')", t, tok, tok_text);
    next();
}

// [English] Converts an lvalue to an rvalue by dereferencing if kind is VAL_LVALUE.
// After conversion, kind is set to VAL_RVALUE.
// [Portuguese] Converte um lvalue para rvalue fazendo dereferência se kind for VAL_LVALUE.
// Após a conversão, kind é definido como VAL_RVALUE.
static void convert_rvalue(int *kind)
{
    if (*kind)
    {
        gen_deref();
        *kind = VAL_RVALUE;
    }
}

// Declarations / Declarações

// [English] Parses function parameters enclosed in parentheses.
// Installs each parameter as SYM_PARAM and assigns sequential offsets.
// [Portuguese] Analisa parâmetros de função entre parênteses.
// Instala cada parâmetro como SYM_PARAM e atribui offsets sequenciais.
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
            symbol_t *s = install(tok_text, SYM_PARAM, 0, SEG_STACK);
            s->offset = func_nparams++;
            next();
            if (tok != TOK_COMMA)
                break;
            next();
        }
    }
    expect(TOK_RPAREN);
}

// [English] Parses variable declarations (auto or extrn), including array declarations.
// Auto variables are installed as SYM_LOCAL on the stack; extrn as SYM_EXTERN in DATA.
// [Portuguese] Analisa declarações de variáveis (auto ou extrn), incluindo declarações de array.
// Variáveis auto são instaladas como SYM_LOCAL na pilha; extrn como SYM_EXTERN em DATA.
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
            install(name, SYM_EXTERN, arr_size, SEG_DATA);
            gen_extern(name);
        }
        else if (is_auto)
        {
            symbol_t *sym = install(name, SYM_LOCAL, arr_size, SEG_STACK);
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

// Expression parsing / Análise de expressões

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
static int parse_args_rtl(long first_pos);

// [English] Parses a primary expression: numbers, character constants, string literals,
// and parenthesized expressions.
// [Portuguese] Analisa uma expressão primária: números, constantes de caractere,
// literais string e expressões entre parênteses.
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
        if (!strcmp(target_cpu, "8086mz"))
            gen_dword(0);
        else
            gen_word(0);
        gen_text();
        gen_load_label(l);
        next();
        kind = VAL_RVALUE;
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

// [English] Parses postfix expressions: array subscript [], function call (),
// increment ++ and decrement --
// [Portuguese] Analisa expressões pós-fixadas: subscrito de array [], chamada de função (),
// incremento ++ e decremento --
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
            gen_push_prim();

            next();
            int idx_kind = expression();
            convert_rvalue(&idx_kind);
            expect(TOK_RBRACKET);

            // HL = index, DE = address (from stack)
            gen_pop_sec();

            // index * 2 (word size)
            gen_double();
            gen_add();
            kind = VAL_LVALUE;
        }
        else if (tok == TOK_LPAREN)
        {
            error("indirect function calls not yet supported");
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
            gen_push_prim();
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

// For function call handling / Para tratamento de chamada de função
static char last_func_name[256];
static char deferred_load_name[256];

// [English] Parses primary expressions with identifier tracking for function calls.
// Identifiers are looked up in the symbol table and generate appropriate load code
// (local, param, or global address). Saves the identifier name for function call resolution.
// [Portuguese] Analisa expressões primárias com rastreamento de identificador para chamadas de função.
// Identificadores são buscados na tabela de símbolos e geram código de carga apropriado
// (local, parâmetro ou endereço global). Salva o nome do identificador para resolução de chamada de função.
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
        if (!strcmp(target_cpu, "8086mz"))
            gen_dword(0);
        else
            gen_word(0);
        gen_text();
        gen_load_label(l);
        next();
        kind = VAL_RVALUE;
    }
    else if (tok == TOK_IDENT)
    {
        strncpy(last_func_name, tok_text, 255);
        last_func_name[255] = '\0';
        symbol_t *sym = lookup(tok_text);
        if (!sym)
        {
            symkind_t sk = SYM_EXTERN;
            if (!strcmp(tok_text, "peekb") || !strcmp(tok_text, "pokeb") ||
                !strcmp(tok_text, "peekw") || !strcmp(tok_text, "pokew"))
                sk = SYM_FUNCTION;
            sym = install(tok_text, sk, 0, SEG_DATA);
            if (sk == SYM_EXTERN)
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
            if (!(!strcmp(sym->name, "peekb") || !strcmp(sym->name, "pokeb") ||
                  !strcmp(sym->name, "peekw") || !strcmp(sym->name, "pokew")))
            {
                strncpy(deferred_load_name, sym->name, 255);
                deferred_load_name[255] = '\0';
            }
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

// [English] Parses postfix expressions with full function call support, including
// inline handling of peekb/pokeb/peekw/pokew built-in functions, array subscript,
// and increment/decrement operators.
// [Portuguese] Analisa expressões pós-fixadas com suporte completo a chamadas de função,
// incluindo tratamento inline das funções embutidas peekb/pokeb/peekw/pokew,
// subscrito de array e operadores de incremento/decremento.
static int postfix_func(void)
{
    int kind = primary_func();
    char funcname[256];
    int has_func_name = 0;

    // Check if this was a simple function name / Verifica se era um nome de função simples
    if (kind == VAL_LVALUE && last_func_name[0])
    {
        symbol_t *sym = lookup(last_func_name);
        if (sym && (sym->kind == SYM_EXTERN || sym->kind == SYM_FUNCTION))
        {
            strcpy(funcname, last_func_name);
            has_func_name = 1;
        }
    }

    // Emit deferred load address or discard if function call
    if (deferred_load_name[0])
    {
        if (tok == TOK_LPAREN && has_func_name)
            deferred_load_name[0] = '\0';
        else
        {
            gen_load_addr(deferred_load_name);
            deferred_load_name[0] = '\0';
        }
    }

    while (1)
    {

        // Array subscript / Subscrito de array
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

        // Function call / Chamada de função
        else if (tok == TOK_LPAREN)
        {
            if (!has_func_name)
                error("function call requires function name");

            // Inline peekb / peekb inline
            if (!strcmp(funcname, "peekb"))
            {
                next();
                int pk = assignment();
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline peekb");
                gen_peekb();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }

            // Inline peekw / peekw inline
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

            // Inline pokeb / pokeb inline
            if (!strcmp(funcname, "pokeb"))
            {
                next();
                int pk = assignment();
                convert_rvalue(&pk);
                gen_push_prim();
                expect(TOK_COMMA);
                pk = assignment();
                convert_rvalue(&pk);
                expect(TOK_RPAREN);
                gen_comment("inline pokeb");
                gen_pop_sec();
                gen_pokeb();
                kind = VAL_RVALUE;
                has_func_name = 0;
                last_func_name[0] = 0;
                break;
            }

            // Inline pokew / pokew inline
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

            // Regular function call / Chamada de função normal
            long args_start;
            {
                int ch = lex_get_ch();
                if (ch != EOF) ungetc(ch, lex_get_fp());
            }
            args_start = ftell(lex_get_fp());
            lex_set_ch(' ');
            next();
            gen_comment("call %s", funcname);
            int nargs = 0;
            if (tok != TOK_RPAREN)
            {
                nargs = parse_args_rtl(args_start);
            }
            expect(TOK_RPAREN);

            gen_comment("call %s(%i args)", funcname, nargs);
            gen_call(funcname, nargs);
            kind = VAL_RVALUE;
            has_func_name = 0;
            last_func_name[0] = 0;
        }

        // Postfix increment / Incremento pós-fixado
        else if (tok == TOK_INC)
        {
            gen_push_prim();
            convert_rvalue(&kind);
            gen_push_prim();
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_add();
            gen_pop_sec();
            gen_store_to_addr();
            gen_pop_sec();
            gen_exchange();
            next();
            kind = VAL_RVALUE;
        }

        // Postfix decrement / Decremento pós-fixado
        else if (tok == TOK_DEC)
        {
            gen_push_prim();
            convert_rvalue(&kind);
            gen_push_prim();
            gen_push_prim();
            gen_load_imm(1);
            gen_pop_sec();
            gen_sub();
            gen_pop_sec();
            gen_store_to_addr();
            gen_pop_sec();
            gen_exchange();
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

// [English] Parses unary expressions: dereference (*), address-of (&), negation (-),
// bitwise not (~), logical not (!), prefix increment (++), prefix decrement (--),
// and falls through to postfix expressions.
// [Portuguese] Analisa expressões unárias: dereferência (*), endereço (&), negação (-),
// não bitwise (~), não lógico (!), pré-incremento (++), pré-decremento (--),
// e passa para expressões pós-fixadas.
static int unary(void)
{
    if (tok == TOK_STAR)
    {
        next();
        int kind = unary();
        return VAL_LVALUE;
    }
    else if (tok == TOK_AMPERSAND)
    {
        next();
        int kind = unary();
        if (!kind)
            error("lvalue required as operand of &");
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
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_add();
        gen_pop_sec();
        gen_store_to_addr();
        return VAL_RVALUE;
    }
    else if (tok == TOK_DEC)
    {
        next();
        int kind = unary();
        if (!kind)
            error("lvalue required as operand of --");
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        gen_load_imm(1);
        gen_pop_sec();
        gen_sub();
        gen_pop_sec();
        gen_store_to_addr();
        return VAL_RVALUE;
    }
    else
    {
        return postfix_func();
    }
}

// [English] Parses multiplicative expressions: *, /, %
// [Portuguese] Analisa expressões multiplicativas: *, /, %
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

// [English] Parses additive expressions: +, -
// [Portuguese] Analisa expressões aditivas: +, -
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

// [English] Parses shift expressions: <<, >>
// [Portuguese] Analisa expressões de deslocamento: <<, >>
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

// [English] Parses relational expressions: <, >, <=, >=
// [Portuguese] Analisa expressões relacionais: <, >, <=, >=
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

// [English] Parses equality expressions: ==, !=
// [Portuguese] Analisa expressões de igualdade: ==, !=
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

// [English] Parses bitwise AND expressions: &
// [Portuguese] Analisa expressões AND bitwise: &
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

// [English] Parses bitwise XOR expressions: ^
// [Portuguese] Analisa expressões XOR bitwise: ^
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

// [English] Parses bitwise OR expressions: |
// [Portuguese] Analisa expressões OR bitwise: |
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

// [English] Parses logical AND expressions with short-circuit evaluation: &&
// [Portuguese] Analisa expressões AND lógico com avaliação de curto-circuito: &&
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

// [English] Parses logical OR expressions with short-circuit evaluation: ||
// [Portuguese] Analisa expressões OR lógico com avaliação de curto-circuito: ||
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

// [English] Parses conditional expressions (currently just a wrapper for logical_or)
// [Portuguese] Analisa expressões condicionais (atualmente apenas um wrapper para logical_or)
int conditional(void)
{
    return logical_or();
}

// [English] Parses assignment expressions: =, +=, -=, *=, /=, %=, &=, |=, ^=
// For simple assignment, pops the address and stores the value.
// For compound assignment, reads the current value, applies the operation, then stores.
// [Portuguese] Analisa expressões de atribuição: =, +=, -=, *=, /=, %=, &=, |=, ^=
// Para atribuição simples, desempilha o endereço e armazena o valor.
// Para atribuição composta, lê o valor atual, aplica a operação, então armazena.
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
        convert_rvalue(&right_kind);
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
        gen_push_prim();
        gen_deref();
        gen_push_prim();
        next();
        int right_kind = assignment();
        gen_pop_sec();

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

        gen_pop_sec();
        gen_store_to_addr();
        return VAL_RVALUE;
    }

    return kind;
}

// [English] Parses comma-separated expressions
// [Portuguese] Analisa expressões separadas por vírgula
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

// Function call args (right-to-left evaluation) / Argumentos de chamada de função (avaliação da direita para a esquerda)

// [English] Parses function call arguments with right-to-left evaluation order.
// Phase 1: muted parse to record argument file positions.
// Phase 2: replays arguments in reverse order with real code generation,
// pushing each argument value onto the stack.
// [Portuguese] Analisa argumentos de chamada de função com ordem de avaliação da direita para a esquerda.
// Fase 1: análise silenciosa para registrar posições dos argumentos no arquivo.
// Fase 2: reproduz argumentos em ordem reversa com geração de código real,
// empilhando cada valor de argumento na pilha.
static int parse_args_rtl(long first_pos)
{
    FILE *fp = lex_get_fp();
    long starts[256];
    int count = 0;
    FILE *saved_out = outfile;

    // Phase 1: muted parse, record argument file positions
    // Fase 1: análise silenciosa, registra posições dos argumentos no arquivo
    outfile = devnull;

    starts[0] = first_pos;
    assignment();
    count = 1;
    while (tok == TOK_COMMA)
    {
        {
            int ch = lex_get_ch();
            if (ch != EOF) ungetc(ch, fp);
        }
        starts[count] = ftell(fp);
        lex_set_ch(' ');
        next();
        assignment();
        count++;
    }
    int saved_ch = lex_get_ch();
    long end_pos = ftell(fp);

    // Phase 2: replay in right-to-left order with real codegen
    // Fase 2: reproduz em ordem da direita para a esquerda com geração de código real
    outfile = saved_out;
    for (int i = count - 1; i >= 0; i--)
    {
        fseek(fp, starts[i], SEEK_SET);
        lex_sync();
        next();
        int kind = assignment();
        convert_rvalue(&kind);
        gen_push_prim();
    }

    // Advance past remaining argument tokens to reach the outer RPAREN
    // Avança além dos tokens de argumento restantes para alcançar o RPAREN externo
    {
        int depth = 0;
        for (;;)
        {
            if (tok == TOK_LPAREN) depth++;
            else if (tok == TOK_RPAREN)
            {
                if (--depth < 0) break;
            }
            next();
        }
    }

    // Restore lexer state so expect(TOK_RPAREN) reads the token after )
    // Restaura o estado do lexer para que expect(TOK_RPAREN) leia o token após )
    fseek(fp, end_pos, SEEK_SET);
    lex_set_ch(saved_ch);

    return count;
}

// Statements / Comandos

// [English] Parses a single statement: asm(), compound {}, if, while, for, do-while,
// return, break, declarations, or expression statements.
// [Portuguese] Analisa um único comando: asm(), composto {}, if, while, for, do-while,
// return, break, declarações ou expressões.
static void parse_statement(void)
{
    // asm() statement / Comando asm()
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

    // Compound statement / Comando composto
    else if (tok == TOK_LBRACE)
    {
        compound_statement();
    }

    // if statement / Comando if
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

    // while statement / Comando while
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

    // for statement / Comando for
    else if (tok == TOK_FOR)
    {
        int l_test = gen_label();
        int l_body = gen_label();
        int l_done = gen_label();
        int saved_break = break_label;
        break_label = l_done;

        next();
        expect(TOK_LPAREN);

        // Init / Inicialização
        if (tok != TOK_SEMICOLON)
        {
            int kind = expression();
        }
        expect(TOK_SEMICOLON);

        gen_label_int(l_test);
        // Test / Teste
        if (tok != TOK_SEMICOLON)
        {
            int kind = expression();
            convert_rvalue(&kind);
            gen_comment("for test");
            gen_jz(l_done);
        }
        expect(TOK_SEMICOLON);

        // Parse increment, emit to temp buffer for later replay
        // Analisa incremento, emite para buffer temporário para reprodução posterior
        FILE *inc_fp = tmpfile();
        FILE *saved_out = outfile;
        outfile = inc_fp;

        if (tok != TOK_RPAREN)
        {
            gen_comment("for increment");
            int inc_kind = expression();
        }
        expect(TOK_RPAREN);

        long inc_size = ftell(inc_fp);
        rewind(inc_fp);

        outfile = saved_out;

        parse_statement();

        // Emit deferred increment code / Emite código de incremento adiado
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

    // do-while statement / Comando do-while
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

    // return statement / Comando return
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

    // break statement / Comando break
    else if (tok == TOK_BREAK)
    {
        next();
        expect(TOK_SEMICOLON);
        if (break_label < 0)
            error("break outside loop");
        gen_jmp(break_label);
    }

    // Empty statement / Comando vazio
    else if (tok == TOK_SEMICOLON)
    {
        next();
    }

    // Declaration / Declaração
    else if (tok == TOK_AUTO || tok == TOK_EXTRN)
    {
        parse_declaration();
    }

    // Expression statement / Expressão como comando
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

// [English] Parses a compound statement enclosed in braces { }
// [Portuguese] Analisa um comando composto entre chaves { }
void compound_statement(void)
{
    expect(TOK_LBRACE);
    while (tok != TOK_RBRACE && tok != TOK_EOF)
    {
        parse_statement();
    }
    expect(TOK_RBRACE);
}

// Top-level / Nível superior

// [English] Parses a function definition: name, parameters, declarations, and body.
// Generates the prologue (stack frame setup), parses all statements,
// then generates the epilogue (stack frame teardown and return).
// [Portuguese] Analisa uma definição de função: nome, parâmetros, declarações e corpo.
// Gera o prólogo (configuração do quadro de pilha), analisa todos os comandos,
// então gera o epílogo (desmontagem do quadro de pilha e retorno).
static void function_definition(const char *name)
{
    gen_global(name);
    gen_text();

    func_nlocals = 0;
    func_nparams = 0;
    func_has_return = 0;
    func_has_return_at_end = 0;
    break_label = -1;

    parse_params();

    expect(TOK_LBRACE);

    // Parse all declarations first to count locals
    // Analisa todas as declarações primeiro para contar variáveis locais
    while (tok == TOK_AUTO || tok == TOK_EXTRN)
    {
        parse_declaration();
    }

    gen_comment("function %s prologue", name);
    gen_prologue(name, func_nlocals);

    // Parse remaining statements / Analisa comandos restantes
    while (tok != TOK_RBRACE && tok != TOK_EOF)
    {
        int is_return = (tok == TOK_RETURN);
        parse_statement();
        func_has_return_at_end = is_return;
    }

    expect(TOK_RBRACE);
    if (!func_has_return_at_end)
    {
        gen_comment("function %s epilogue", name);
        gen_epilogue();
    }
}

// [English] Parses the entire compilation unit (top level).
// Handles global variable declarations with initial values in DATA or BSS segments,
// and function definitions.
// [Portuguese] Analisa a unidade de compilação inteira (nível superior).
// Processa declarações de variáveis globais com valores iniciais nos segmentos DATA ou BSS,
// e definições de função.
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
                if (tok == TOK_LBRACKET) {
                    // Array -> BSS / Array -> BSS
                    next();
                    int size = tok_value;
                    if (tok != TOK_NUMBER) error("array size expected");
                    next();
                    expect(TOK_RBRACKET);
                    symbol_t *sym = install(name, SYM_GLOBAL, size, SEG_BSS);
                    gen_data();
                    gen_global(name);
                    gen_bss();
                    gen_label_str(name);
                    gen_reserve(size * 2);
                } else {
                    // Scalar -> DATA / Escalar -> DATA
                    symbol_t *sym = install(name, SYM_GLOBAL, 0, SEG_DATA);
                    gen_data();
                    gen_global(name);
                    gen_label_str(name);
                    if (!strcmp(target_cpu, "8086mz"))
                        gen_dword(0);
                    else
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
