#include "bcomp.h"

token_t tok;
char tok_text[256];
int tok_value;
int line_num = 1;
int col_num = 1;
const char *filename = NULL;

static FILE *src_fp = NULL;
static int ch = ' ';
static int peek = 0;

// [English] Initializes the lexer with a file pointer, resets character and
// peek buffer, and sets line/column counters to 1
// [Portuguese] Inicializa o analisador léxico com um ponteiro de arquivo,
// redefine o buffer de caracteres e peek, e define contadores de linha/coluna para 1
void lex_init(FILE *fp)
{
    src_fp = fp;
    ch = ' ';
    peek = 0;
    line_num = 1;
    col_num = 1;
}

// [English] Returns the current source file pointer used by the lexer
// [Portuguese] Retorna o ponteiro do arquivo fonte atual usado pelo analisador léxico
FILE *lex_get_fp(void)
{
    return src_fp;
}

// [English] Resets the lexer state: sets current character to space and clears peek buffer
// [Portuguese] Redefine o estado do analisador léxico: define caractere atual como espaço e limpa o buffer peek
void lex_sync(void)
{
    ch = ' ';
    peek = 0;
}

// [English] Returns the current character in the lexer buffer
// [Portuguese] Retorna o caractere atual no buffer do analisador léxico
int lex_get_ch(void)
{
    return ch;
}

// [English] Sets the current character and clears the peek buffer
// [Portuguese] Define o caractere atual e limpa o buffer peek
void lex_set_ch(int c)
{
    ch = c;
    peek = 0;
}

// [English] Reads the next character from the source file, handling peek buffer.
// Updates line and column counters.
// [Portuguese] Lê o próximo caractere do arquivo fonte, gerenciando o buffer peek.
// Atualiza os contadores de linha e coluna.
static int next_char(void)
{
    int c;
    if (peek)
    {
        c = peek;
        peek = 0;
    }
    else
    {
        c = fgetc(src_fp);
    }
    if (c == '\n')
    {
        line_num++;
        col_num = 1;
    }
    else if (c != EOF)
    {
        col_num++;
    }
    return c;
}

// [English] Pushes back a character into the peek buffer for later retrieval
// [Portuguese] Devolve um caractere para o buffer peek para recuperação posterior
static void unget_char(int c)
{
    peek = c;
}

// [English] Checks if a character can start an identifier (letter or underscore)
// [Portuguese] Verifica se um caractere pode iniciar um identificador (letra ou sublinhado)
static int is_ident_start(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// [English] Checks if a character can continue an identifier (letter, digit, or underscore)
// [Portuguese] Verifica se um caractere pode continuar um identificador (letra, dígito ou sublinhado)
static int is_ident_cont(int c)
{
    return is_ident_start(c) || (c >= '0' && c <= '9');
}

// [English] Checks if a character is an octal digit (0-7)
// [Portuguese] Verifica se um caractere é um dígito octal (0-7)
static int is_octal(int c)
{
    return c >= '0' && c <= '7';
}

// [English] Checks if a character is a hexadecimal digit (0-9, a-f, A-F)
// [Portuguese] Verifica se um caractere é um dígito hexadecimal (0-9, a-f, A-F)
static int is_hex(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// [English] Converts a hexadecimal character to its numeric value (0-15)
// [Portuguese] Converte um caractere hexadecimal para seu valor numérico (0-15)
static int hex_val(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return c - 'A' + 10;
}

// [English] Skips whitespace characters (space, tab, carriage return, newline)
// by reading characters until a non-whitespace character is found
// [Portuguese] Pula caracteres de espaço em branco (espaço, tabulação, retorno de carro, nova linha)
// lendo caracteres até que um caractere não-branco seja encontrado
static void skip_whitespace(void)
{
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
    {
        ch = next_char();
    }
}

// [English] Checks if the given string matches a B language keyword.
// Returns the corresponding token type or TOK_IDENT if not a keyword.
// [Portuguese] Verifica se a string fornecida corresponde a uma palavra-chave da linguagem B.
// Retorna o tipo de token correspondente ou TOK_IDENT se não for uma palavra-chave.
static token_t find_keyword(const char *s)
{
    if (!strcmp(s, "if"))
        return TOK_IF;
    if (!strcmp(s, "else"))
        return TOK_ELSE;
    if (!strcmp(s, "while"))
        return TOK_WHILE;
    if (!strcmp(s, "for"))
        return TOK_FOR;
    if (!strcmp(s, "do"))
        return TOK_DO;
    if (!strcmp(s, "return"))
        return TOK_RETURN;
    if (!strcmp(s, "break"))
        return TOK_BREAK;
    if (!strcmp(s, "auto"))
        return TOK_AUTO;
    if (!strcmp(s, "extrn"))
        return TOK_EXTRN;
    if (!strcmp(s, "asm"))
        return TOK_ASM;
    return TOK_IDENT;
}

// [English] Reads a character constant (e.g. 'A') and handles escape sequences.
// Sets tok_value to the character's numeric value.
// [Portuguese] Lê uma constante de caractere (ex.: 'A') e processa sequências de escape.
// Define tok_value com o valor numérico do caractere.
static token_t read_char(void)
{
    if (ch == EOF)
        error("unterminated character constant");
    if (ch == '\\')
    {
        ch = next_char();
        if (ch == 'n')
            tok_value = '\n';
        else if (ch == 'r')
            tok_value = '\r';
        else if (ch == 't')
            tok_value = '\t';
        else if (ch == '0')
            tok_value = '\0';
        else if (ch == '\\')
            tok_value = '\\';
        else if (ch == '\'')
            tok_value = '\'';
        else if (ch == '"')
            tok_value = '"';
        else
            tok_value = ch;
    }
    else
    {
        tok_value = ch;
    }
    ch = next_char();
    if (ch != '\'')
        error("expected '");
    ch = next_char();
    return TOK_CHAR;
}

// [English] Reads a string constant between double quotes, handling escape sequences.
// Stores the parsed string in tok_text with a maximum length of 254 characters.
// [Portuguese] Lê uma constante string entre aspas duplas, processando sequências de escape.
// Armazena a string analisada em tok_text com comprimento máximo de 254 caracteres.
static token_t read_string(void)
{
    int len = 0;
    if (ch == '"')
        ch = next_char();
    while (1)
    {
        if (ch == EOF)
            error("unterminated string");
        if (ch == '"')
        {
            ch = next_char();
            break;
        }
        if (ch == '\\')
        {
            ch = next_char();
            if (ch == 'n')
                tok_text[len++] = '\n';
            else if (ch == 'r')
                tok_text[len++] = '\r';
            else if (ch == 't')
                tok_text[len++] = '\t';
            else if (ch == '0')
                tok_text[len++] = '\0';
            else if (ch == '\\')
                tok_text[len++] = '\\';
            else if (ch == '"')
                tok_text[len++] = '"';
            else
                tok_text[len++] = ch;
        }
        else
        {
            tok_text[len++] = ch;
        }
        if (len >= 254)
            error("string too long");
        ch = next_char();
    }
    tok_text[len] = '\0';
    return TOK_STRING;
}

// [English] Main tokenizer function: reads the next token from the source input.
// Handles comments (// and /* */), identifiers, keywords, numbers (decimal/hex/octal),
// character/string constants, and all operators/punctuation.
// [Portuguese] Função principal do analisador léxico: lê o próximo token da entrada fonte.
// Processa comentários (// e /* */), identificadores, palavras-chave, números (decimal/hex/octal),
// constantes caractere/string e todos os operadores/pontuação.
void next_token(void)
{
    skip_whitespace();

    tok_text[0] = '\0';
    tok_value = 0;

    // Handle comments / Processa comentários
    while (ch == '/')
    {
        int next = fgetc(src_fp);
        if (next == '/')
        {
            // Line comment / Comentário de linha
            while (ch != '\n' && ch != EOF)
                ch = next_char();
            skip_whitespace();
        }
        else if (next == '*')
        {
            // Block comment / Comentário de bloco
            ch = next_char();
            while (1)
            {
                if (ch == EOF)
                    error("unterminated comment");
                if (ch == '*')
                {
                    ch = next_char();
                    if (ch == '/')
                    {
                        ch = next_char();
                        break;
                    }
                }
                else
                {
                    ch = next_char();
                }
            }
            skip_whitespace();
        }
        else
        {
            unget_char(next);
            break;
        }
    }

    if (ch == EOF)
    {
        tok = TOK_EOF;
        return;
    }

    // Character constant / Constante de caractere
    if (ch == '\'')
    {
        ch = next_char();
        tok = read_char();
        return;
    }

    // String constant / Constante string
    if (ch == '"')
    {
        tok = read_string();
        return;
    }

    // Identifier or keyword / Identificador ou palavra-chave
    if (is_ident_start(ch))
    {
        int len = 0;
        while (is_ident_cont(ch))
        {
            if (len < 254)
                tok_text[len++] = ch;
            ch = next_char();
        }
        tok_text[len] = '\0';
        tok = find_keyword(tok_text);
        return;
    }

    // Number (decimal, hex, octal) / Número (decimal, hexadecimal, octal)
    if (ch >= '0' && ch <= '9')
    {
        tok = TOK_NUMBER;
        tok_value = 0;
        if (ch == '0')
        {
            ch = next_char();
            if (ch == 'x' || ch == 'X')
            {
                // Hex / Hexadecimal
                ch = next_char();
                while (is_hex(ch))
                {
                    tok_value = (tok_value << 4) | hex_val(ch);
                    ch = next_char();
                }
            }
            else if (is_octal(ch))
            {
                // Octal
                while (is_octal(ch))
                {
                    tok_value = (tok_value << 3) + (ch - '0');
                    ch = next_char();
                }
            }
        }
        else
        {
            // Decimal
            while (ch >= '0' && ch <= '9')
            {
                tok_value = tok_value * 10 + (ch - '0');
                ch = next_char();
            }
        }
        return;
    }

    // Operators and punctuation / Operadores e pontuação
    tok = TOK_EOF;
    switch (ch)
    {
    case '+':
        ch = next_char();
        if (ch == '+')
        {
            tok = TOK_INC;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_ADD_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_PLUS;
        return;
    case '-':
        ch = next_char();
        if (ch == '-')
        {
            tok = TOK_DEC;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_SUB_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_MINUS;
        return;
    case '*':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_MUL_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_STAR;
        return;
    case '/':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_DIV_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_SLASH;
        return;
    case '%':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_MOD_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_PERCENT;
        return;
    case '&':
        ch = next_char();
        if (ch == '&')
        {
            tok = TOK_AND;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_AND_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_AMPERSAND;
        return;
    case '|':
        ch = next_char();
        if (ch == '|')
        {
            tok = TOK_OR;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_OR_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_PIPE;
        return;
    case '^':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_XOR_ASSIGN;
            ch = next_char();
        }
        else
            tok = TOK_CARET;
        return;
    case '~':
        ch = next_char();
        tok = TOK_TILDE;
        return;
    case '!':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_NE;
            ch = next_char();
        }
        else
            tok = TOK_BANG;
        return;
    case '<':
        ch = next_char();
        if (ch == '<')
        {
            tok = TOK_LSHIFT;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_LE;
            ch = next_char();
        }
        else
            tok = TOK_LT;
        return;
    case '>':
        ch = next_char();
        if (ch == '>')
        {
            tok = TOK_RSHIFT;
            ch = next_char();
        }
        else if (ch == '=')
        {
            tok = TOK_GE;
            ch = next_char();
        }
        else
            tok = TOK_GT;
        return;
    case '=':
        ch = next_char();
        if (ch == '=')
        {
            tok = TOK_EQ;
            ch = next_char();
        }
        else
            tok = TOK_ASSIGN;
        return;
    case '(':
        ch = next_char();
        tok = TOK_LPAREN;
        return;
    case ')':
        ch = next_char();
        tok = TOK_RPAREN;
        return;
    case '[':
        ch = next_char();
        tok = TOK_LBRACKET;
        return;
    case ']':
        ch = next_char();
        tok = TOK_RBRACKET;
        return;
    case '{':
        ch = next_char();
        tok = TOK_LBRACE;
        return;
    case '}':
        ch = next_char();
        tok = TOK_RBRACE;
        return;
    case ';':
        ch = next_char();
        tok = TOK_SEMICOLON;
        return;
    case ',':
        ch = next_char();
        tok = TOK_COMMA;
        return;
    case ':':
        ch = next_char();
        tok = TOK_COLON;
        return;
    case '.':
        ch = next_char();
        tok = TOK_DOT;
        return;
    default:
        error("unknown character: '%c' (%i)", ch, ch);
    }
}

// [English] Wrapper function that calls next_token() to advance to the next token
// [Portuguese] Função wrapper que chama next_token() para avançar para o próximo token
void next(void)
{
    next_token();
}
