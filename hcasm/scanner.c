#include "asm.h"

// [English] Current and look-ahead token pointers (for single-token look-ahead)
// [Portuguese] Ponteiros de token atual e look-ahead (para look-ahead de um token)
static expr_t *_current = NULL;
static expr_t *_next = NULL;

// [English] Macro: read current source char into a 1-char string, append to token buffer
// [Portuguese] Macro: lê char atual do fonte em uma string de 1 char, anexa ao buffer do token
#define CAT()             \
    c[0] = source_getc(); \
    strncat(token, c, 255);

// [English] Macro: read current char into token, then advance source
// [Portuguese] Macro: lê char atual no token, então avança o fonte
#define CATNEXT() \
    CAT();        \
    source_nextc();

// [English] Convert a hex character (0-9, a-f, A-F) to its numeric value (0-15)
// [Portuguese] Converte um caractere hexadecimal (0-9, a-f, A-F) para seu valor numérico (0-15)
static inline int hex_val(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return c - 'A' + 10;
}

// [English] Macro: read escaped char, append to token
// [Portuguese] Macro: lê caractere escapado, anexa ao token
#define CATESC()                \
    c[0] = source_getescapec(); \
    strncat(token, c, 255);

// [English] Macro: read escaped char, append to token, advance source
// [Portuguese] Macro: lê caractere escapado, anexa ao token, avança fonte
#define CATESCNEXT() \
    CATESC();        \
    source_nextc();

// [English] Check if current token matches a specific token type
// [Portuguese] Verifica se o token atual corresponde a um tipo específico
bool curr_is(token_t token)
{
    return _current->token == token;
}

// [English] Case-insensitive comparison: current token text vs a keyword
// [Portuguese] Comparação case-insensitive: texto do token atual vs uma palavra-chave
bool curr_is_keyword(char *keyword)
{
    char *s1 = _current->text;
    char *s2 = keyword;
    while (tolower(*s1) == tolower(*s2))
    {
        if (*s1 == 0)
            return true;
        s1++;
        s2++;
    }
    return false;
}

// [English] Case-insensitive comparison: arbitrary token text vs a keyword
// [Portuguese] Comparação case-insensitive: texto de token arbitrário vs uma palavra-chave
bool is_keyword(char *token, char *keyword)
{
    char *s1 = token;
    char *s2 = keyword;
    while (tolower(*s1) == tolower(*s2))
    {
        if (*s1 == 0)
            return true;
        s1++;
        s2++;
    }
    return false;
}

// [English] Check if the look-ahead token matches a specific type
// [Portuguese] Verifica se o token de look-ahead corresponde a um tipo específico
bool next_is(token_t token)
{
    return _next->token == token;
}

// [English] Return the current token (error if none)
// [Portuguese] Retorna o token atual (erro se não houver)
expr_t *curr()
{
    if (!_current)
        error("current token not exists");
    return _current;
}

// [English] Deep-clone an expression tree node (copies the struct + flexible text array)
// [Portuguese] Clona profundamente um nó da árvore de expressão (copia a struct + array de texto flexível)
expr_t *clone_expr(expr_t *src)
{
    expr_t *e = malloc(sizeof(expr_t) + strlen(src->text));
    memcpy(e, src, sizeof(expr_t) + strlen(src->text));
    return e;
}

// [English] Recursively free an expression tree
// [Portuguese] Libera recursivamente uma árvore de expressão
void free_expr(expr_t *e)
{
    if (!e)
        return;
    if (e->left)
        free_expr(e->left);
    if (e->right)
        free_expr(e->right);
    free(e);
}

// [English] Scan the next token from source and return the previous current.
// [Portuguese] Escaneia o próximo token do fonte e retorna o anterior atual.
// [English] Handles: numbers (dec/bin/oct/hex), symbols, strings, registers, operators, brackets.
// [Portuguese] Processa: números (dec/bin/oct/hex), símbolos, strings, registradores, operadores, colchetes.
// [English] Uses single-token look-ahead: _next holds the upcoming token.
// [Portuguese] Usa look-ahead de um token: _next contém o próximo token.
expr_t *scan()
{
    char c[2];
    c[1] = 0;
    expr_t e;
    reg_t *reg;
    char token[256];
    strcpy(token, "");
    memset(&e, 0, sizeof(expr_t));

    // [English] Free old current token, advance look-ahead to become current
    // [Portuguese] Libera token atual antigo, avança look-ahead para se tornar atual
    if (_current != NULL)
    {
        free_expr(_current);
    }
    if (_next != NULL)
    {
        _current = _next;
    }
    _next = NULL;

    // [English] Skip whitespace (spaces, tabs, carriage returns) and comments (semicolons)
    // [Portuguese] Pula espaços em branco (espaços, tabulações, retornos de carro) e comentários (ponto e vírgula)
    while (source_is(' ') || source_is('\t') || source_is('\r') || source_is(';'))
    {
        if (source_is(';'))
        {
            // [English] Comment: skip to end of line
            // [Portuguese] Comentário: pula até o fim da linha
            while (!source_is('\n') && !source_is(0))
                source_nextc();
        }
        else
            source_nextc();
    }

    e.line = _source->line;
    e.column = _source->column;
    e.filename = _source->filename;

    // [English] --== Numeric literal starting with '0' ==--
    // [Portuguese] --== Literal numérico começando com '0' ==--
    // [English] Supports: 0b... (binary), 0o... (octal), 0x... (hex), 0... (octal), ...h (hex suffix)
    // [Portuguese] Suporta: 0b... (binário), 0o... (octal), 0x... (hexadecimal), 0... (octal), ...h (sufixo hex)
    if (source_is('0'))
    {
        e.token = TOK_VALUE;
        CATNEXT();
        // [English] 0b prefix -> binary literal
        // [Portuguese] Prefixo 0b -> literal binário
        if (source_is('b'))
        {
            CATNEXT();
            while (source_between('0', '1'))
            {
                e.value <<= 1;
                e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        // [English] 0o prefix -> octal literal
        // [Portuguese] Prefixo 0o -> literal octal
        else if (source_is('o'))
        {
            CATNEXT();
            while (source_between('0', '7'))
            {
                e.value <<= 3;
                e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        // [English] 0x prefix -> hexadecimal literal
        // [Portuguese] Prefixo 0x -> literal hexadecimal
        else if (source_is('x'))
        {
            CATNEXT();
            while (source_between('0', '9') || source_between('a', 'f') || source_between('A', 'F'))
            {
                e.value <<= 4;
                if (source_between('a', 'f'))
                    e.value += source_getc() - 'a' + 10;
                else if (source_between('A', 'F'))
                    e.value += source_getc() - 'A' + 10;
                else
                    e.value += source_getc() - '0';
                CATNEXT();
            }
        }
        // [English] 0... without prefix -> octal if all digits 0-7, otherwise hex with H suffix or zero
        // [Portuguese] 0... sem prefixo -> octal se todos dígitos 0-7, senão hex com sufixo H ou zero
        else
        {
            bool has_non_octal = false;
            int hex_count = 0;
            while (source_between('0', '9') || source_between('a', 'f') || source_between('A', 'F'))
            {
                if (!source_between('0', '7'))
                    has_non_octal = true;
                CATNEXT();
                hex_count++;
            }
            // [English] H suffix -> treat as hex
            // [Portuguese] Sufixo H -> trata como hexadecimal
            if (source_is('h') || source_is('H'))
            {
                CATNEXT();
                char *p = token;
                e.value = 0;
                while (*p && *p != 'h' && *p != 'H')
                {
                    e.value = (e.value << 4) | hex_val(*p);
                    p++;
                }
            }
            // [English] All octal digits -> octal value
            // [Portuguese] Todos dígitos octais -> valor octal
            else if (!has_non_octal && hex_count > 0)
            {
                e.value = 0;
                char *p = token;
                while (*p)
                {
                    e.value <<= 3;
                    e.value += *p - '0';
                    p++;
                }
            }
            // [English] Invalid/empty -> default to 0
            // [Portuguese] Inválido/vazio -> padrão 0
            else
            {
                strcpy(token, "0");
                e.value = 0;
            }
        }
    }

    // [English] --== Decimal number starting with 1-9 ==--
    // [Portuguese] --== Número decimal começando com 1-9 ==--
    // [English] Also detects optional 'h' hex suffix (e.g. 0A5h)
    // [Portuguese] Também detecta sufixo hex 'h' opcional (ex.: 0A5h)
    else if (source_between('0', '9'))
    {
        e.token = TOK_VALUE;
        while (source_between('0', '9'))
        {
            e.value *= 10;
            e.value += source_getc() - '0';
            CATNEXT();
        }
        if (source_is('h') || source_is('H'))
        {
            CATNEXT();
            char *p = token;
            e.value = 0;
            while (*p && *p != 'h' && *p != 'H')
            {
                e.value = (e.value << 4) | hex_val(*p);
                p++;
            }
        }
    }

    // [English] --== Identifier / symbol / label reference ==--
    // [Portuguese] --== Identificador / símbolo / referência de rótulo ==--
    // [English] May also be a register name, hex value with H suffix, sub-label (.name), or directive (%ifdef, etc)
    // [Portuguese] Pode também ser nome de registrador, valor hex com sufixo H, sub-rótulo (.nome), ou diretiva (%ifdef, etc)
    else if (source_between('0', '9') || source_between('a', 'z') || source_between('A', 'Z') || source_is('_') || source_is('.') || source_is('%'))
    {
        e.token = TOK_SYMBOL;
        // [English] First character may be %, for directives like %ifdef
        // [Portuguese] Primeiro caractere pode ser %, para diretivas como %ifdef
        if (source_is('%'))
        {
            CATNEXT();
        }
        // [English] Rest of identifier (letters, digits, underscores, dots)
        // [Portuguese] Resto do identificador (letras, dígitos, sublinhados, pontos)
        while (source_between('0', '9') || source_between('a', 'z') || source_between('A', 'Z') || source_is('_') || source_is('.'))
        {
            CATNEXT();
        }
        // [English] Optional tick suffix (e.g. for alternate register set: af')
        // [Portuguese] Sufixo de apóstrofo opcional (ex.: para conjunto alternativo de registradores: af')
        if (source_is('\''))
        {
            CATNEXT();
        }
        // [English] Check if this identifier matches a CPU register name
        // [Portuguese] Verifica se este identificador corresponde a um nome de registrador da CPU
        reg = _regs;
        while (reg->name)
        {
            if (is_keyword(reg->name, token))
            {
                e.reg = reg;
                e.token = TOK_REGISTER;
                break;
            }
            reg++;
        }
        // [English] If still a symbol, check for hex literal with H suffix (e.g. 0FFh)
        // [Portuguese] Se ainda é um símbolo, verifica literal hex com sufixo H (ex.: 0FFh)
        if (e.token == TOK_SYMBOL)
        {
            int tlen = strlen(token);
            if (tlen > 1 && (token[tlen - 1] == 'h' || token[tlen - 1] == 'H'))
            {
                bool is_hex = true;
                for (int i = 0; i < tlen - 1; i++)
                {
                    char c = token[i];
                    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                    {
                        is_hex = false;
                        break;
                    }
                }
                if (is_hex)
                {
                    e.token = TOK_VALUE;
                    e.value = 0;
                    for (int i = 0; i < tlen - 1; i++)
                        e.value = (e.value << 4) | hex_val(token[i]);
                }
            }
        }
        // [English] Dot prefix -> local sub-label, prepend current label name
        // [Portuguese] Prefixo ponto -> sub-rótulo local, precede com nome do rótulo atual
        if (token[0] == '.')
        {
            if (_current_label)
            {
                char *label = malloc(strlen(token) + strlen(_current_label) + 1);
                strcpy(label, _current_label);
                strcat(label, token);
                strncpy(token, label, 255);
                token[255] = 0;
                free(label);
            }
            e.token = TOK_SUB_LABEL;
        }
    }

    // [English] --== Character literal (single quotes) ==--
    // [Portuguese] --== Literal de caractere (aspas simples) ==--
    // [English] Supports escape sequences. Multi-char stored big-endian.
    // [Portuguese] Suporta sequências de escape. Multi-caractere armazenado big-endian.
    else if (source_is('\''))
    {
        e.token = TOK_VALUE;
        source_nextc();
        while (!source_is('\'') && !source_is(0))
        {
            e.value <<= 8;
            e.value |= source_getescapec();
            CATNEXT();
        }
        if (!source_is('\''))
            error("\"'\" expected.");
        source_nextc();
    }

    // [English] --== String literal (double quotes) ==--
    // [Portuguese] --== Literal de string (aspas duplas) ==--
    else if (source_is('"'))
    {
        e.token = TOK_STRING;
        source_nextc();
        while (!source_is('"') && !source_is(0))
        {
            CATESCNEXT();
        }
        if (!source_is('"'))
            error("'\"' expected.");
        source_nextc();
    }

    // [English] --== Single-character operators and delimiters ==--
    // [Portuguese] --== Operadores e delimitadores de um caractere ==--
    else if (source_is('<'))
    {
        e.token = TOK_LOBYTE;
        CATNEXT();
    }
    else if (source_is('>'))
    {
        e.token = TOK_HIBYTE;
        CATNEXT();
    }
    else if (source_is('+'))
    {
        e.token = TOK_ADD;
        CATNEXT();
    }
    else if (source_is('-'))
    {
        e.token = TOK_SUB;
        CATNEXT();
    }
    else if (source_is('/'))
    {
        e.token = TOK_DIV;
        CATNEXT();
    }
    else if (source_is('%'))
    {
        e.token = TOK_MOD;
        CATNEXT();
    }
    else if (source_is('*'))
    {
        e.token = TOK_MUL;
        CATNEXT();
    }
    else if (source_is('#'))
    {
        e.token = TOK_HASH;
        CATNEXT();
    }
    else if (source_is(':'))
    {
        e.token = TOK_COLON;
        CATNEXT();
    }
    else if (source_is('$'))
    {
        e.token = TOK_CURRENT_POS;
        CATNEXT();
    }
    else if (source_is(','))
    {
        e.token = TOK_COMMA;
        CATNEXT();
    }
    else if (source_is('('))
    {
        e.token = TOK_PARAMS_OPEN;
        CATNEXT();
    }
    else if (source_is(')'))
    {
        e.token = TOK_PARAMS_CLOSE;
        CATNEXT();
    }
    else if (source_is('['))
    {
        e.token = TOK_INDEX_OPEN;
        CATNEXT();
    }
    else if (source_is(']'))
    {
        e.token = TOK_INDEX_CLOSE;
        CATNEXT();
    }
    else if (source_is('\n'))
    {
        e.token = TOK_NEWLINE;
        CATNEXT();
    }
    else if (source_is(0))
    {
        e.token = TOK_EOF;
    }
    else
        error("unknown char: '%c' (%i)", source_getc(), source_getc());

    // [English] Allocate next token, copy scanned data, store as look-ahead
    // [Portuguese] Aloca próximo token, copia dados escaneados, armazena como look-ahead
    _next = malloc(sizeof(expr_t) + strlen(token));
    memcpy(_next, &e, sizeof(expr_t));
    strcpy(_next->text, token);
    return _current;
}
