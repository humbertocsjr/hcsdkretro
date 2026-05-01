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

void lex_init(FILE *fp)
{
    src_fp = fp;
    ch = ' ';
    peek = 0;
    line_num = 1;
    col_num = 1;
}

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

static void unget_char(int c)
{
    peek = c;
}

static int is_ident_start(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_ident_cont(int c)
{
    return is_ident_start(c) || (c >= '0' && c <= '9');
}

static int is_octal(int c)
{
    return c >= '0' && c <= '7';
}

static int is_hex(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hex_val(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return c - 'A' + 10;
}

static void skip_whitespace(void)
{
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
    {
        ch = next_char();
    }
}

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

void next_token(void)
{
    skip_whitespace();

    tok_text[0] = '\0';
    tok_value = 0;

    while (ch == '/')
    {
        int next = fgetc(src_fp);
        if (next == '/')
        {
            while (ch != '\n' && ch != EOF)
                ch = next_char();
            skip_whitespace();
        }
        else if (next == '*')
        {
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

    if (ch == '\'')
    {
        ch = next_char(); /* advance past opening ' */
        tok = read_char();
        return;
    }

    if (ch == '"')
    {
        tok = read_string();
        return;
    }

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

    if (ch >= '0' && ch <= '9')
    {
        tok = TOK_NUMBER;
        tok_value = 0;
        if (ch == '0')
        {
            ch = next_char();
            if (ch == 'x' || ch == 'X')
            {
                ch = next_char();
                while (is_hex(ch))
                {
                    tok_value = (tok_value << 4) | hex_val(ch);
                    ch = next_char();
                }
            }
            else if (is_octal(ch))
            {
                while (is_octal(ch))
                {
                    tok_value = (tok_value << 3) + (ch - '0');
                    ch = next_char();
                }
            }
        }
        else
        {
            while (ch >= '0' && ch <= '9')
            {
                tok_value = tok_value * 10 + (ch - '0');
                ch = next_char();
            }
        }
        return;
    }

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

void next(void)
{
    next_token();
}
