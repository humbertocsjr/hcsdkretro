#include "bcomp.h"
#include "ast.h"

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

// AST-based expression parsing / Análise de expressões baseada em AST

static ast_node_t *parse_unary_ast(void);
static ast_node_t *parse_term_ast(void);
static ast_node_t *parse_additive_ast(void);
static ast_node_t *parse_shift_ast(void);
static ast_node_t *parse_relational_ast(void);
static ast_node_t *parse_equality_ast(void);
static ast_node_t *parse_bitwise_and_ast(void);
static ast_node_t *parse_bitwise_xor_ast(void);
static ast_node_t *parse_bitwise_or_ast(void);
static ast_node_t *parse_logical_and_ast(void);
static ast_node_t *parse_logical_or_ast(void);
static ast_node_t *parse_conditional_ast(void);
static ast_node_t *parse_assignment_ast(void);
static ast_node_t *parse_expression_ast(void);

static ast_node_t *parse_primary_ast(void)
{
    if (tok == TOK_NUMBER)
    {
        ast_node_t *node = ast_int(tok_value);
        next();
        return node;
    }
    else if (tok == TOK_CHAR)
    {
        ast_node_t *node = ast_char(tok_value);
        next();
        return node;
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
        return ast_string(tok_text);
    }
    else if (tok == TOK_LPAREN)
    {
        next();
        ast_node_t *node = parse_expression_ast();
        expect(TOK_RPAREN);
        return node;
    }
    else if (tok == TOK_IDENT)
    {
        symbol_t *sym = lookup(tok_text);
        if (!sym)
        {
            sym = install(tok_text, SYM_EXTERN, 0, SEG_DATA);
            gen_extern(tok_text);
        }
        ast_node_t *node = ast_ident(sym);
        next();
        return node;
    }
    else
    {
        error("expression expected, got '%s'", tok_text);
        return NULL;
    }
}

static ast_node_t *parse_postfix_ast(void)
{
    ast_node_t *node = parse_primary_ast();
    
    while (1)
    {
        if (tok == TOK_LBRACKET)
        {
            next();
            ast_node_t *idx = parse_expression_ast();
            expect(TOK_RBRACKET);
            node = ast_index(node, idx);
        }
        else if (tok == TOK_LPAREN)
        {
            next();
            ast_node_t *args = NULL;
            if (tok != TOK_RPAREN)
            {
                args = parse_expression_ast();
                while (tok == TOK_COMMA)
                {
                    next();
                    ast_node_t *next_arg = parse_expression_ast();
                    args = ast_binary(AST_COMMA, args, next_arg);
                }
            }
            expect(TOK_RPAREN);
            node = ast_call(node, args);
        }
        else if (tok == TOK_INC)
        {
            next();
            node = ast_unary(AST_POST_INC, node);
        }
        else if (tok == TOK_DEC)
        {
            next();
            node = ast_unary(AST_POST_DEC, node);
        }
        else
        {
            break;
        }
    }
    
    return node;
}

static ast_node_t *parse_unary_ast(void)
{
    if (tok == TOK_MINUS)
    {
        next();
        return ast_unary(AST_NEG, parse_unary_ast());
    }
    else if (tok == TOK_TILDE)
    {
        next();
        return ast_unary(AST_NOT, parse_unary_ast());
    }
    else if (tok == TOK_BANG)
    {
        next();
        return ast_unary(AST_LNOT, parse_unary_ast());
    }
    else if (tok == TOK_AMPERSAND)
    {
        next();
        return ast_unary(AST_ADDR, parse_unary_ast());
    }
    else if (tok == TOK_STAR)
    {
        next();
        return ast_unary(AST_DEREF, parse_unary_ast());
    }
    else if (tok == TOK_INC)
    {
        next();
        return ast_unary(AST_PRE_INC, parse_unary_ast());
    }
    else if (tok == TOK_DEC)
    {
        next();
        return ast_unary(AST_PRE_DEC, parse_unary_ast());
    }
    else
    {
        return parse_postfix_ast();
    }
}

static ast_node_t *parse_term_ast(void)
{
    ast_node_t *node = parse_unary_ast();
    
    while (tok == TOK_STAR || tok == TOK_SLASH || tok == TOK_PERCENT)
    {
        ast_op_t op;
        if (tok == TOK_STAR) op = AST_MUL;
        else if (tok == TOK_SLASH) op = AST_DIV;
        else op = AST_MOD;
        
        next();
        ast_node_t *right = parse_unary_ast();
        node = ast_binary(op, node, right);
    }
    
    return node;
}

static ast_node_t *parse_additive_ast(void)
{
    ast_node_t *node = parse_term_ast();
    
    while (tok == TOK_PLUS || tok == TOK_MINUS)
    {
        ast_op_t op = (tok == TOK_PLUS) ? AST_ADD : AST_SUB;
        next();
        ast_node_t *right = parse_term_ast();
        node = ast_binary(op, node, right);
    }
    
    return node;
}

static ast_node_t *parse_shift_ast(void)
{
    ast_node_t *node = parse_additive_ast();
    
    while (tok == TOK_LSHIFT || tok == TOK_RSHIFT)
    {
        ast_op_t op = (tok == TOK_LSHIFT) ? AST_SHL : AST_SHR;
        next();
        ast_node_t *right = parse_additive_ast();
        node = ast_binary(op, node, right);
    }
    
    return node;
}

static ast_node_t *parse_relational_ast(void)
{
    ast_node_t *node = parse_shift_ast();
    
    while (tok == TOK_LT || tok == TOK_GT || tok == TOK_LE || tok == TOK_GE)
    {
        ast_op_t op;
        if (tok == TOK_LT) op = AST_LT;
        else if (tok == TOK_GT) op = AST_GT;
        else if (tok == TOK_LE) op = AST_LE;
        else op = AST_GE;
        
        next();
        ast_node_t *right = parse_shift_ast();
        node = ast_binary(op, node, right);
    }
    
    return node;
}

static ast_node_t *parse_equality_ast(void)
{
    ast_node_t *node = parse_relational_ast();
    
    while (tok == TOK_EQ || tok == TOK_NE)
    {
        ast_op_t op = (tok == TOK_EQ) ? AST_EQ : AST_NE;
        next();
        ast_node_t *right = parse_relational_ast();
        node = ast_binary(op, node, right);
    }
    
    return node;
}

static ast_node_t *parse_bitwise_and_ast(void)
{
    ast_node_t *node = parse_equality_ast();
    
    while (tok == TOK_AMPERSAND || tok == TOK_AND)
    {
        next();
        ast_node_t *right = parse_equality_ast();
        node = ast_binary(AST_AND, node, right);
    }
    
    return node;
}

static ast_node_t *parse_bitwise_xor_ast(void)
{
    ast_node_t *node = parse_bitwise_and_ast();
    
    while (tok == TOK_CARET)
    {
        next();
        ast_node_t *right = parse_bitwise_and_ast();
        node = ast_binary(AST_XOR, node, right);
    }
    
    return node;
}

static ast_node_t *parse_bitwise_or_ast(void)
{
    ast_node_t *node = parse_bitwise_xor_ast();
    
    while (tok == TOK_PIPE || tok == TOK_OR)
    {
        next();
        ast_node_t *right = parse_bitwise_xor_ast();
        node = ast_binary(AST_OR, node, right);
    }
    
    return node;
}

static ast_node_t *parse_logical_and_ast(void)
{
    return parse_bitwise_or_ast();
}

static ast_node_t *parse_logical_or_ast(void)
{
    return parse_logical_and_ast();
}

static ast_node_t *parse_conditional_ast(void)
{
    return parse_logical_or_ast();
}

static ast_node_t *parse_assignment_ast(void)
{
    ast_node_t *node = parse_conditional_ast();
    
    if (tok == TOK_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_ASSIGN, node, right);
    }
    else if (tok == TOK_ADD_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_ADD_ASSIGN, node, right);
    }
    else if (tok == TOK_SUB_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_SUB_ASSIGN, node, right);
    }
    else if (tok == TOK_MUL_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_MUL_ASSIGN, node, right);
    }
    else if (tok == TOK_DIV_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_DIV_ASSIGN, node, right);
    }
    else if (tok == TOK_MOD_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_MOD_ASSIGN, node, right);
    }
    else if (tok == TOK_AND_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_AND_ASSIGN, node, right);
    }
    else if (tok == TOK_OR_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_OR_ASSIGN, node, right);
    }
    else if (tok == TOK_XOR_ASSIGN)
    {
        next();
        ast_node_t *right = parse_assignment_ast();
        node = ast_binary(AST_XOR_ASSIGN, node, right);
    }
    
    return node;
}

static ast_node_t *parse_expression_ast(void)
{
    return parse_assignment_ast();
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
int expression(void)
{
    ast_node_t *ast = parse_expression_ast();
    ast_node_t *opt = ast_optimize(ast);
    
    // Generate code from AST
    ast_gen(opt);
    
    // Free memory properly
    if (opt != ast)
        ast_free(ast);
    ast_free(opt);
    
    return VAL_RVALUE;
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

    outfile = devnull;

    starts[0] = first_pos;
    parse_expression_ast();
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
        parse_expression_ast();
        count++;
    }
    int saved_ch = lex_get_ch();
    long end_pos = ftell(fp);

    outfile = saved_out;
    for (int i = count - 1; i >= 0; i--)
    {
        fseek(fp, starts[i], SEEK_SET);
        lex_sync();
        next();
        ast_node_t *ast = parse_expression_ast();
        ast_node_t *opt = ast_optimize(ast);
        ast_gen(opt);
        if (opt != ast)
            ast_free(ast);
        ast_free(opt);
        gen_push_prim();
    }

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
            ast_node_t *ast = parse_expression_ast();
            ast_gen_rvalue(ast);
            ast_free(ast);
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
