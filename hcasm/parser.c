#include "asm.h"

char *_current_label = NULL;
cond_ctx_t *_cond_stack = NULL;  // [English] Stack of conditional contexts
                                  // [Portuguese] Pilha de contextos condicionais

expr_t *parse_expr();

// [English] Push a new conditional context onto the stack
// [Portuguese] Coloca um novo contexto condicional na pilha
static void cond_push(bool condition)
{
    cond_ctx_t *ctx = malloc(sizeof(cond_ctx_t));
    ctx->active = condition;
    ctx->any_branch_active = condition;  // [English] Mark if this first branch is active
                                         // [Portuguese] Marca se este primeiro ramo está ativo
    ctx->else_seen = false;
    ctx->next = _cond_stack;
    _cond_stack = ctx;
}

// [English] Pop a conditional context from the stack and return it
// [Portuguese] Remove um contexto condicional da pilha e o retorna
static cond_ctx_t *cond_pop()
{
    if (!_cond_stack)
        error("unexpected %%endif without matching %%ifdef/%%ifndef");
    cond_ctx_t *ctx = _cond_stack;
    _cond_stack = ctx->next;
    return ctx;
}

// [English] Get the current conditional context without removing it
// [Portuguese] Obtém o contexto condicional atual sem removê-lo
static cond_ctx_t *cond_current()
{
    return _cond_stack;
}

// [English] Check if current line should be processed (true if all parent conditions are active)
// [Portuguese] Verifica se a linha atual deve ser processada (verdadeiro se todos os pais estão ativos)
static bool cond_is_active()
{
    cond_ctx_t *ctx = _cond_stack;
    while (ctx)
    {
        if (!ctx->active)
            return false;
        ctx = ctx->next;
    }
    return true;
}

// [English] Check if the current token matches a prefix name (from _prefix[] table)
// [Portuguese] Verifica se o token atual corresponde a um nome de prefixo (da tabela _prefix[])
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

// [English] Check if the current token matches a mnemonic name (from _opcode[] table)
// [Portuguese] Verifica se o token atual corresponde a um nome de mnemônico (da tabela _opcode[])
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

// [English] Look up and return the prefix opcode entry matching the current token
// [Portuguese] Procura e retorna a entrada de opcode de prefixo correspondente ao token atual
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

// [English] Look up and return the instruction opcode entry matching the current token
// [Portuguese] Procura e retorna a entrada de opcode de instrução correspondente ao token atual
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

// [English] Parse a primary expression (lowest precedence):
// [Portuguese] Analisa uma expressão primária (menor precedência):
// [English]   unary minus, value/register/symbol, sub-label, unary operators (< >),
// [Portuguese]   menos unário, valor/registrador/símbolo, sub-rótulo, operadores unários (< >),
// [English]   bracket expressions, parenthesized expressions
// [Portuguese]   expressões entre colchetes, expressões entre parênteses
expr_t *parse_expr0()
{
    expr_t *e = NULL;
    if (curr_is(TOK_SUB))
    {
        // [English] Unary minus: synthesize 0 - expr
        // [Portuguese] Menos unário: sintetiza 0 - expr
        e = clone_expr(curr());
        e->left = clone_expr(curr());
        e->left->token = TOK_VALUE;
        e->left->value = 0;
        scan();
        e->right = parse_expr0();
    }
    // [English] Atoms: value, register, symbol, current position ($)
    // [Portuguese] Átomos: valor, registrador, símbolo, posição atual ($)
    else if (curr_is(TOK_VALUE) || curr_is(TOK_REGISTER) || curr_is(TOK_SYMBOL) || curr_is(TOK_CURRENT_POS))
    {
        e = clone_expr(curr());
        scan();
    }
    // [English] Sub-label (.name) -- convert to a full symbol token
    // [Portuguese] Sub-rótulo (.nome) -- converte para um token de símbolo completo
    else if (curr_is(TOK_SUB_LABEL))
    {
        e = clone_expr(curr());
        e->token = TOK_SYMBOL;
        scan();
    }
    // [English] Low-byte / high-byte unary operators
    // [Portuguese] Operadores unários low-byte / high-byte
    else if (curr_is(TOK_LOBYTE) || curr_is(TOK_HIBYTE))
    {
        e = clone_expr(curr());
        scan();
        e->right = parse_expr();
    }
    // [English] Index expression [...] -- for memory addressing
    // [Portuguese] Expressão de índice [...] -- para endereçamento de memória
    else if (curr_is(TOK_INDEX_OPEN))
    {
        e = clone_expr(curr());
        scan();
        e->right = parse_expr();
        if (!curr_is(TOK_INDEX_CLOSE))
            error_expr(curr(), "']' expected [Token: '%s'(#%i)]", curr()->text, curr()->token);
        scan();
    }
    // [English] Parenthesized expression (...)
    // [Portuguese] Expressão entre parênteses (...)
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

// [English] Expression level 1: pass-through to parse_expr0
// [Portuguese] Nível 1 de expressão: pass-through para parse_expr0
expr_t *parse_expr1()
{
    return parse_expr0();
}

// [English] Expression level 2: multiplicative operators (*, /, %)
// [Portuguese] Nível 2 de expressão: operadores multiplicativos (*, /, %)
// [English] Left-associative, higher precedence than addition
// [Portuguese] Associativo à esquerda, precedência maior que adição
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

// [English] Expression level 3: additive operators (+, -) and segment override (:)
// [Portuguese] Nível 3 de expressão: operadores aditivos (+, -) e override de segmento (:)
// [English] Left-associative for +/-, right-associative for :
// [Portuguese] Associativo à esquerda para +/-, associativo à direita para :
// [English] Register + offset is parsed greedily for indexed addressing
// [Portuguese] Registrador + offset é analisado greedy para endereçamento indexado
expr_t *parse_expr3()
{
    expr_t *e = parse_expr2();
    // [English] If the left side is a register, allow register + offset for indexed addressing
    // [Portuguese] Se o lado esquerdo é um registrador, permite registrador + offset para endereçamento indexado
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
        // [English] Standard left-associative addition/subtraction
        // [Portuguese] Adição/subtração padrão associativa à esquerda
        while (curr_is(TOK_ADD) || curr_is(TOK_SUB))
        {
            expr_t *op = clone_expr(curr());
            scan();
            op->left = e;
            op->right = parse_expr2();
            e = op;
        }
    // [English] Segment override operator (e.g., es:bx)
    // [Portuguese] Operador de override de segmento (ex.: es:bx)
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

// [English] Top-level expression parser: parse then constant-fold
// [Portuguese] Analisador de expressão de topo: analisa então dobra constantes
expr_t *parse_expr()
{
    return optimize(parse_expr3());
}

// [English] Parse a single line of assembly source.
// [Portuguese] Analisa uma única linha de código fonte assembly.
// [English] Handles: instruction mnemonics, prefixes, labels, data directives (db/dw/dd),
// [Portuguese] Processa: mnemônicos de instrução, prefixos, rótulos, diretivas de dados (db/dw/dd),
// [English] global/extern declarations, section changes, reserve directives, equ, times.
// [Portuguese] declarações global/extern, mudanças de seção, diretivas de reserva, equ, times.
void parse_line()
{
    expr_t *mnemonic = NULL;
    int argc = 0;
    expr_t *argv[ARGV_MAX];
    
    // [English] Process conditional directives (%ifdef, %ifndef, %else, %endif)
    // [Portuguese] Processa diretivas condicionais (%ifdef, %ifndef, %else, %endif)
    // [English] These directives MUST be processed regardless of current condition state
    // [Portuguese] Estas diretivas DEVEM ser processadas independentemente do estado condicional atual
    if (curr_is(TOK_SYMBOL))
    {
        if (curr_is_keyword("%ifdef"))
        {
            expr_t *directive = clone_expr(curr());
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(directive, "%%ifdef requires a symbol name");
            char name[256];
            strncpy(name, curr()->text, 255);
            name[255] = 0;
            bool defined = consts_exists(name);
            scan();
            cond_push(defined);
            free_expr(directive);
            return;
        }
        else if (curr_is_keyword("%ifndef"))
        {
            expr_t *directive = clone_expr(curr());
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(directive, "%%ifndef requires a symbol name");
            char name[256];
            strncpy(name, curr()->text, 255);
            name[255] = 0;
            bool defined = consts_exists(name);
            scan();
            cond_push(!defined);
            free_expr(directive);
            return;
        }
        else if (curr_is_keyword("%elifdef"))
        {
            expr_t *directive = clone_expr(curr());
            cond_ctx_t *ctx = cond_current();
            if (!ctx)
                error_expr(directive, "%%elifdef without matching %%ifdef/%%ifndef");
            
            if (ctx->else_seen)
                error_expr(directive, "%%elifdef after %%else");
            
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(directive, "%%elifdef requires a symbol name");
            char name[256];
            strncpy(name, curr()->text, 255);
            name[255] = 0;
            bool defined = consts_exists(name);
            
            // [English] If a branch was already active, this one is not
            // [Portuguese] Se um ramo já estava ativo, este não é
            if (ctx->any_branch_active)
            {
                ctx->active = false;
            }
            else
            {
                // [English] No active branch yet; activate if this symbol is defined
                // [Portuguese] Nenhum ramo ativo ainda; ativa se este símbolo está definido
                ctx->active = defined;
                if (defined)
                    ctx->any_branch_active = true;
            }
            scan();
            free_expr(directive);
            return;
        }
        else if (curr_is_keyword("%elifndef"))
        {
            expr_t *directive = clone_expr(curr());
            cond_ctx_t *ctx = cond_current();
            if (!ctx)
                error_expr(directive, "%%elifndef without matching %%ifdef/%%ifndef");
            
            if (ctx->else_seen)
                error_expr(directive, "%%elifndef after %%else");
            
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(directive, "%%elifndef requires a symbol name");
            char name[256];
            strncpy(name, curr()->text, 255);
            name[255] = 0;
            bool defined = consts_exists(name);
            
            // [English] If a branch was already active, this one is not
            // [Portuguese] Se um ramo já estava ativo, este não é
            if (ctx->any_branch_active)
            {
                ctx->active = false;
            }
            else
            {
                // [English] No active branch yet; activate if this symbol is NOT defined
                // [Portuguese] Nenhum ramo ativo ainda; ativa se este símbolo NÃO está definido
                ctx->active = !defined;
                if (!defined)
                    ctx->any_branch_active = true;
            }
            scan();
            free_expr(directive);
            return;
        }
        else if (curr_is_keyword("%else"))
        {
            expr_t *directive = clone_expr(curr());
            cond_ctx_t *ctx = cond_current();
            if (!ctx)
                error_expr(directive, "%%else without matching %%ifdef/%%ifndef");
            if (ctx->else_seen)
                error_expr(directive, "multiple %%else in same block");
            
            // [English] %else is active only if no previous branch was active
            // [Portuguese] %else está ativo apenas se nenhum ramo anterior estava ativo
            ctx->active = !ctx->any_branch_active;
            ctx->any_branch_active = true;  // [English] Mark that a branch is now active
                                            // [Portuguese] Marca que um ramo agora está ativo
            ctx->else_seen = true;
            scan();
            free_expr(directive);
            return;
        }
        else if (curr_is_keyword("%endif"))
        {
            expr_t *directive = clone_expr(curr());
            cond_ctx_t *ctx = cond_pop();
            if (!ctx)
                error_expr(directive, "%%endif without matching %%ifdef/%%ifndef");
            free(ctx);
            scan();
            free_expr(directive);
            return;
        }
    }
    
    // [English] If current condition is not active, skip the rest of the line
    // [Portuguese] Se a condição atual não está ativa, pula o resto da linha
    if (!cond_is_active())
    {
        // [English] Skip tokens until we reach NEWLINE or EOF
        // [Portuguese] Pula tokens até atingir NEWLINE ou EOF
        while (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
            scan();
        return;
    }
    
    // [English] Check for data directives FIRST, before the prefix loop.
    // [Portuguese] Verifica diretivas de dados PRIMEIRO, antes do loop de prefixos.
    // [English] This handles ambiguity: 'ds' is both a segment prefix and a data reserve directive.
    // [Portuguese] Isso trata a ambiguidade: 'ds' é tanto um prefixo de segmento quanto uma diretiva de reserva de dados.
    if ((curr_is(TOK_SYMBOL) || curr_is(TOK_SUB_LABEL) || curr_is(TOK_REGISTER)) &&
        (curr_is_keyword("db") || curr_is_keyword("dw") || curr_is_keyword("dd") ||
         curr_is_keyword("ds") || curr_is_keyword("rb") || curr_is_keyword("resb") ||
         curr_is_keyword("rw") || curr_is_keyword("resw") ||
         curr_is_keyword("global") || curr_is_keyword("extern") ||
         curr_is_keyword("section") || curr_is_keyword("times")))
    {
        goto parse_label;
    }
    
    // [English] Process CPU prefixes (e.g., segment overrides, rep, lock)
    // [Portuguese] Processa prefixos de CPU (ex.: overrides de segmento, rep, lock)
    while (is_prefix(curr()))
    {
        opcode_t *op = parse_prefix(curr());
        mnemonic = clone_expr(curr());
        scan();
        op->emit(mnemonic, op, 0, argv);
    }

    // [English] Label definition: symbol followed by colon
    // [Portuguese] Definição de rótulo: símbolo seguido de dois-pontos
    if (curr_is(TOK_SYMBOL) && next_is(TOK_COLON))
    {
        goto parse_label;
    }

    // [English] Instruction mnemonic
    // [Portuguese] Mnemônico de instrução
    if (is_mnemonic(curr()))
    {
        opcode_t *op = parse_mnemonic(curr());
        mnemonic = clone_expr(curr());
        scan();

        // [English] Parse size/distance prefixes (byte, word, dword, qword, short, near, far)
        // [Portuguese] Analisa prefixos de tamanho/distância (byte, word, dword, qword, short, near, far)
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
            // [English] Immediate value prefix (#) -- marks operand as immediate
            // [Portuguese] Prefixo de valor imediato (#) -- marca operando como imediato
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
            // [English] Arguments are comma-separated
            // [Portuguese] Argumentos são separados por vírgula
            if (!curr_is(TOK_COMMA))
                break;
            scan();
        }
        // [English] Emit the instruction via the opcode table's handler
        // [Portuguese] Emite a instrução via o manipulador da tabela de opcodes
        op->emit(mnemonic, op, argc, argv);
        for (int i = 0; i < argc; i++)
        {
            free_expr(argv[i]);
        }
    }

    // [English] Directives and label definitions
    // [Portuguese] Diretivas e definições de rótulo
    else if (curr_is(TOK_SYMBOL) || curr_is(TOK_SUB_LABEL) || curr_is(TOK_REGISTER))
    {
    parse_label:
        // [English] DB -- Define byte(s)
        // [Portuguese] DB -- Define byte(s)
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
        // [English] DW -- Define word(s) (16-bit)
        // [Portuguese] DW -- Define word(s) (16-bit)
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
        // [English] DD -- Define double-word(s) (32-bit, stores as two 16-bit words)
        // [Portuguese] DD -- Define double-word(s) (32-bit, armazena como duas palavras de 16-bit)
        else if (curr_is_keyword("dd"))
        {
            scan();
            int v = 0;
            while (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
            {
                switch (curr()->token)
                {
                case TOK_VALUE:
                    v = curr()->value;
                    out(REC_DATA, 0, 0, &v, 2);
                    v = curr()->value >> 16;
                    out(REC_DATA, 0, 0, &v, 2);
                    scan();
                    break;
                case TOK_STRING:
                    out(REC_DATA, 0, 0, curr()->text, strlen(curr()->text));
                    scan();
                    break;
                default:;
                    expr_t *arg = parse_expr();
                    out(generate(arg, 0, false) ? REC_EXPR_POP_INT16_RELOCATABLE_EMIT : REC_EXPR_POP_INT16_EMIT, 0, 0, 0, 0);
                    v = 0;
                    out(REC_DATA, 0, 0, &v, 2);
                    free_expr(arg);
                    break;
                }
                if (!curr_is(TOK_COMMA))
                    break;
                scan();
            }
        }
        // [English] GLOBAL -- export label to global symbol table
        // [Portuguese] GLOBAL -- exporta rótulo para tabela de símbolos global
        else if (curr_is_keyword("global"))
        {
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(curr(), "name expected.");
            out(REC_CONST_AS_GLOBAL_LABEL, 0, 0, curr()->text, strlen(curr()->text));
            scan();
        }
        // [English] EXTERN -- declare external symbol (consumed but no record emitted)
        // [Portuguese] EXTERN -- declara símbolo externo (consumido mas nenhum registro emitido)
        else if (curr_is_keyword("extern"))
        {
            scan();
            if (!curr_is(TOK_SYMBOL))
                error_expr(curr(), "name expected.");
            scan();
        }
        // [English] SECTION -- switch to text/data/bss section
        // [Portuguese] SECTION -- alterna para seção text/data/bss
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
        // [English] RESB / RB / DS -- reserve bytes
        // [Portuguese] RESB / RB / DS -- reserva bytes
        else if (curr_is_keyword("resb") || curr_is_keyword("rb") || curr_is_keyword("ds"))
        {
            scan();
            argv[0] = parse_expr();
            if (argv[0]->token != TOK_VALUE)
                error_expr(argv[0], "constant expression expected.");
            out(REC_DATA_RESERVE, argv[0]->value, 0, 0, 0);
            free_expr(argv[0]);
        }
        // [English] RESW / RW -- reserve words
        // [Portuguese] RESW / RW -- reserva words
        else if (curr_is_keyword("resw") || curr_is_keyword("rw"))
        {
            scan();
            argv[0] = parse_expr();
            if (argv[0]->token != TOK_VALUE)
                error_expr(argv[0], "constant expression expected.");
            out(REC_DATA_RESERVE, argv[0]->value * 2, 0, 0, 0);
            free_expr(argv[0]);
        }
        // [English] TIMES -- repeat the next instruction/data N times
        // [Portuguese] TIMES -- repete a próxima instrução/dado N vezes
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
        // [English] Label definition or EQU constant
        // [Portuguese] Definição de rótulo ou constante EQU
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
            
            // [English] Check for EQU without colon requirement
            // [Portuguese] Verifica EQU sem requerimento de dois-pontos
            if (curr_is_keyword("equ"))
            {
                // [English] EQU constant definition: NAME equ EXPRESSION
                // [Portuguese] Definição de constante EQU: NOME equ EXPRESSÃO
                scan();
                expr_t *e = parse_expr();
                if (e->token != TOK_VALUE)
                    error_expr(e, "invalid constant expression");
                consts_set(name->text, e->value);
                // [English] Use unsigned variant for values >= INT16_MAX to avoid sign extension in linker
                // [Portuguese] Usa variante sem sinal para valores >= INT16_MAX para evitar extensão de sinal no linker
                out(e->value >= INT16_MAX ? REC_CONST_CUSTOM_UNSIGNED : REC_CONST_CUSTOM, e->value, 0, name->text, strlen(name->text));
                free_expr(name);
                free_expr(e);
            }
            // [English] Label with colon: LABEL: [instructions...]
            // [Portuguese] Rótulo com dois-pontos: RÓTULO: [instruções...]
            else if (curr_is(TOK_COLON))
            {
                scan();
                // [English] Regular label: emit label record, then parse what follows on the same line
                // [Portuguese] Rótulo normal: emite registro de rótulo, então analisa o que segue na mesma linha
                out(REC_CONST_LABEL, 0, 0, name->text, strlen(name->text));
                parse_line();
                free_expr(name);
                return;
            }
            // [English] Error: neither EQU nor colon found after identifier
            // [Portuguese] Erro: nem EQU nem dois-pontos encontrados após identificador
            else
            {
                error_expr(curr(), "':' or 'equ' expected after '%s'.", name->text);
            }
        }
    }
    free_expr(mnemonic);
    if (curr_is(TOK_SYMBOL))
        error_expr(curr(), "mnemonic expected. [Found: '%s'(#%i)]", curr()->text, curr()->token);
    if (!curr_is(TOK_NEWLINE) && !curr_is(TOK_EOF))
        error_expr(curr(), "new line expected. [Found: '%s'(#%i)]", curr()->text, curr()->token);
}

// [English] Top-level parse entry point: open source file, emit headers, parse each line.
// [Portuguese] Ponto de entrada de análise de topo: abre arquivo fonte, emite cabeçalhos, analisa cada linha.
void parse(char *filename)
{
    _cond_stack = NULL;  // [English] Initialize conditional stack
                         // [Portuguese] Inicializa pilha condicional
    source_open(filename);
    scan();
    scan();  // [English] Two scans to fill current + look-ahead
             // [Portuguese] Duas varreduras para preencher atual + look-ahead
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
    
    // [English] Check that all %ifdef/%ifndef blocks are closed with %endif
    // [Portuguese] Verifica que todos os blocos %ifdef/%ifndef estão fechados com %endif
    if (_cond_stack)
        error("unclosed %%ifdef/%%ifndef/%%else block (missing %%endif)");
    
    out(REC_END_OF_FILE, 0, 0, filename, strlen(filename));
    source_close();
}
