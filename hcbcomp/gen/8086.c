#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

// [English] Generates a new unique label number for 8086 assembly
// [Portuguese] Gera um novo número de rótulo único para assembly 8086
int gen_label(void)
{
    return label_counter++;
}

void gen_text(void) { fprintf(out, "\nsection text\n"); }
void gen_data(void) { fprintf(out, "\nsection data\n"); }
void gen_bss(void) { fprintf(out, "\nsection data\n"); }
void gen_global(const char *name) { fprintf(out, "global %s\n", name); }
void gen_extern(const char *name) { fprintf(out, "extern %s\n", name); }
void gen_label_str(const char *name) { fprintf(out, "%s:\n", name); }
void gen_label_int(int label) { fprintf(out, ".L%i:\n", label); }
void gen_word(int val) { fprintf(out, "\tdw %i\n", val); }
void gen_dword(int val) { fprintf(out, "\tdw %i, 0\n", val & 0xFFFF); }

// [English] Emits a string as a DB directive with escape sequence handling for 8086
// [Portuguese] Emite uma string como diretiva DB com tratamento de sequências de escape para 8086
void gen_bytes(const char *str)
{
    fprintf(out, "\tdb \"");
    for (const char *p = str; *p; p++)
    {
        if (*p == '"')
            fprintf(out, "\\\"");
        else if (*p == '\\')
            fprintf(out, "\\\\");
        else if (*p == '\n')
            fprintf(out, "\\n");
        else if (*p == '\r')
            fprintf(out, "\\r");
        else if (*p == '\t')
            fprintf(out, "\\t");
        else
            fputc(*p, out);
    }
    fprintf(out, "\"\n");
}
void gen_reserve(int n) { fprintf(out, "\tds %i\n", n); }

// [English] Emits a formatted comment line in 8086 assembly
// [Portuguese] Emite uma linha de comentário formatada em assembly 8086
void gen_comment(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t; ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

// [English] Emits a raw 8086 assembly instruction line
// [Portuguese] Emite uma linha de instrução assembly 8086 bruta
void gen_emit_raw(const char *line)
{
    fprintf(out, "\t%s\n", line);
}

static void gen_emit(const char *line)
{
    fprintf(out, "\t%s\n", line);
}

static void gen_emitf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

// [English] Generates function prologue for 8086: push bp, mov bp/sp, allocate locals
// [Portuguese] Gera prólogo de função para 8086: push bp, mov bp/sp, aloca locais
void gen_prologue(const char *name, int nlocals)
{
    gen_label_str(name);
    gen_emit("push bp");
    gen_emit("mov bp, sp");
    if (nlocals > 0)
    {
        gen_emitf("sub sp, %i", nlocals * 2);
    }
}

// [English] Generates function epilogue for 8086: mov sp/bp, pop bp, ret
// [Portuguese] Gera epílogo de função para 8086: mov sp/bp, pop bp, ret
void gen_epilogue(void)
{
    gen_emit("mov sp, bp");
    gen_emit("pop bp");
    gen_emit("ret");
}

void gen_return(void)
{
    gen_epilogue();
}

void gen_push_prim(void)
{
    gen_emit("push ax");
}

void gen_pop_sec(void)
{
    gen_emit("pop bx");
}

void gen_load_imm(int val)
{
    gen_emitf("mov ax, %i", val);
}

void gen_load_var(const char *name)
{
    gen_emitf("mov ax, [%s]", name);
}

void gen_load_addr(const char *name)
{
    gen_emitf("mov ax, %s", name);
}

void gen_load_label(int label) { fprintf(outfile, "\tmov ax, .L%i\n", label); }

// [English] Stores AX to a local variable at BP-offset
// [Portuguese] Armazena AX em uma variável local no deslocamento BP-offset
void gen_store_local(int offset)
{
    gen_emitf("mov [bp-%i], ax", offset * 2 + 2);
}

// [English] Loads AX from a local variable at BP-offset
// [Portuguese] Carrega AX de uma variável local no deslocamento BP-offset
void gen_load_local(int offset)
{
    gen_emitf("mov ax, [bp-%i]", offset * 2 + 2);
}

// [English] Computes address of a local variable: LEA AX, [BP-offset]
// [Portuguese] Computa endereço de variável local: LEA AX, [BP-offset]
void gen_local_addr(int offset)
{
    gen_emitf("lea ax, [bp-%i]", offset * 2 + 2);
}

// [English] Stores AX to a parameter at BP+offset
// [Portuguese] Armazena AX em um parâmetro no deslocamento BP+offset
void gen_store_param(int offset)
{
    gen_emitf("mov [bp+%i], ax", offset * 2 + 4);
}

// [English] Loads AX from a parameter at BP+offset
// [Portuguese] Carrega AX de um parâmetro no deslocamento BP+offset
void gen_load_param(int offset)
{
    gen_emitf("mov ax, [bp+%i]", offset * 2 + 4);
}

// [English] Computes address of a parameter: LEA AX, [BP+offset]
// [Portuguese] Computa endereço de parâmetro: LEA AX, [BP+offset]
void gen_param_addr(int offset)
{
    gen_emitf("lea ax, [bp+%i]", offset * 2 + 4);
}

// [English] Stores AX to a named global variable
// [Portuguese] Armazena AX em uma variável global nomeada
void gen_store_global(const char *name)
{
    gen_emitf("mov [%s], ax", name);
}

// [English] Loads AX from a named global variable
// [Portuguese] Carrega AX de uma variável global nomeada
void gen_load_global(const char *name)
{
    gen_emitf("mov ax, [%s]", name);
}

// [English] Dereferences AX: loads the 16-bit value at [AX] into AX via BX
// [Portuguese] Dereferencia AX: carrega o valor de 16 bits em [AX] para AX via BX
void gen_deref(void)
{
    gen_emit("mov bx, ax");
    gen_emit("mov ax, [bx]");
}

// [English] Stores AX to the address in BX
// [Portuguese] Armazena AX no endereço em BX
void gen_store_to_addr(void)
{
    gen_emit("mov [bx], ax");
}

// [English] Reads a byte from [AX] (peekb), zero-extends to 16 bits in AX
// [Portuguese] Lê um byte de [AX] (peekb), estende com zero para 16 bits em AX
void gen_peekb(void)
{
    gen_emit("mov bx, ax");
    gen_emit("xor ah, ah");
    gen_emit("mov al, [bx]");
}

// [English] Writes a byte (pokeb): stores AL to the address in BX
// [Portuguese] Escreve um byte (pokeb): armazena AL no endereço em BX
void gen_pokeb(void)
{
    gen_emit("mov [bx], al");
}

// [English] 16-bit addition: AX = AX + BX
// [Portuguese] Adição de 16 bits: AX = AX + BX
void gen_add(void)
{
    gen_emit("add ax, bx");
}

// [English] Doubles AX: AX = AX * 2
// [Portuguese] Dobra AX: AX = AX * 2
void gen_double(void)
{
    gen_emit("add ax, ax");
}

// [English] 16-bit subtraction: AX = BX - AX (after xchg)
// [Portuguese] Subtração de 16 bits: AX = BX - AX (após xchg)
void gen_sub(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("sub ax, bx");
}

// [English] 16-bit unsigned multiplication: AX = BX * AX
// [Portuguese] Multiplicação unsigned de 16 bits: AX = BX * AX
void gen_mul(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("mul bx");
}

// [English] 16-bit unsigned division: AX = BX / AX, remainder in DX
// [Portuguese] Divisão unsigned de 16 bits: AX = BX / AX, resto em DX
void gen_div(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("cwd");
    gen_emit("div bx");
}

// [English] 16-bit unsigned modulo: DX = BX % AX (result in AX after mov)
// [Portuguese] Módulo unsigned de 16 bits: DX = BX % AX (resultado em AX após mov)
void gen_mod(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("cwd");
    gen_emit("div bx");
    gen_emit("mov ax, dx");
}

// [English] 16-bit negation: AX = -AX
// [Portuguese] Negação de 16 bits: AX = -AX
void gen_neg(void)
{
    gen_emit("neg ax");
}

// [English] 16-bit bitwise NOT: AX = ~AX
// [Portuguese] NOT bitwise de 16 bits: AX = ~AX
void gen_not(void)
{
    gen_emit("not ax");
}

// [English] 16-bit logical NOT: AX = !AX (returns 0 or 1)
// [Portuguese] NOT lógico de 16 bits: AX = !AX (retorna 0 ou 1)
void gen_lnot(void)
{
    int l1 = gen_label();
    int l2 = gen_label();
    gen_emit("or ax, ax");
    gen_emitf("jnz .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", l2);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(l2);
}

// [English] 16-bit bitwise AND: AX = AX & BX
// [Portuguese] AND bitwise de 16 bits: AX = AX & BX
void gen_and(void)
{
    gen_emit("and ax, bx");
}

// [English] 16-bit bitwise OR: AX = AX | BX
// [Portuguese] OR bitwise de 16 bits: AX = AX | BX
void gen_or(void)
{
    gen_emit("or ax, bx");
}

// [English] 16-bit bitwise XOR: AX = AX ^ BX
// [Portuguese] XOR bitwise de 16 bits: AX = AX ^ BX
void gen_xor(void)
{
    gen_emit("xor ax, bx");
}

// [English] 16-bit left shift: shifts BX left by CX times, result in AX
// [Portuguese] Deslocamento à esquerda de 16 bits: desloca BX para esquerda CX vezes, resultado em AX
void gen_shl(void)
{
    int l1 = gen_label();
    int l2 = gen_label();
    gen_emit("mov cx, ax");
    gen_emit("or cx, cx");
    gen_emitf("jz .L%i", l2);
    gen_label_int(l1);
    gen_emit("shl bx, 1");
    gen_emit("dec cx");
    gen_emitf("jnz .L%i", l1);
    gen_label_int(l2);
    gen_emit("mov ax, bx");
}

// [English] 16-bit right shift: shifts BX right by CX times, result in AX
// [Portuguese] Deslocamento à direita de 16 bits: desloca BX para direita CX vezes, resultado em AX
void gen_shr(void)
{
    int l1 = gen_label();
    int l2 = gen_label();
    gen_emit("mov cx, ax");
    gen_emit("or cx, cx");
    gen_emitf("jz .L%i", l2);
    gen_label_int(l1);
    gen_emit("shr bx, 1");
    gen_emit("dec cx");
    gen_emitf("jnz .L%i", l1);
    gen_label_int(l2);
    gen_emit("mov ax, bx");
}

// [English] 16-bit equality comparison: AX = (BX == AX) ? 1 : 0
// [Portuguese] Comparação de igualdade de 16 bits: AX = (BX == AX) ? 1 : 0
void gen_cmp_eq(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("jne .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

// [English] 16-bit not-equal comparison: AX = (BX != AX) ? 1 : 0
// [Portuguese] Comparação de desigualdade de 16 bits: AX = (BX != AX) ? 1 : 0
void gen_cmp_ne(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("je .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

// [English] 16-bit less-than comparison: AX = (BX < AX) ? 1 : 0
// [Portuguese] Comparação menor-que de 16 bits: AX = (BX < AX) ? 1 : 0
void gen_cmp_lt(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("jnl .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

// [English] 16-bit greater-than comparison: AX = (BX > AX) ? 1 : 0
// [Portuguese] Comparação maior-que de 16 bits: AX = (BX > AX) ? 1 : 0
void gen_cmp_gt(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("jng .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

// [English] 16-bit less-or-equal comparison: AX = (BX <= AX) ? 1 : 0
// [Portuguese] Comparação menor-ou-igual de 16 bits: AX = (BX <= AX) ? 1 : 0
void gen_cmp_le(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("jg .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

// [English] 16-bit greater-or-equal comparison: AX = (BX >= AX) ? 1 : 0
// [Portuguese] Comparação maior-ou-igual de 16 bits: AX = (BX >= AX) ? 1 : 0
void gen_cmp_ge(void)
{
    int l1 = gen_label();
    int lend = gen_label();
    gen_emit("cmp bx, ax");
    gen_emitf("jl .L%i", l1);
    gen_emit("mov ax, 1");
    gen_emitf("jmp .L%i", lend);
    gen_label_int(l1);
    gen_emit("mov ax, 0");
    gen_label_int(lend);
}

void gen_jmp(int label)
{
    gen_emitf("jmp .L%i", label);
}

void gen_jz(int label)
{
    gen_emit("or ax, ax");
    gen_emitf("jz near .L%i", label);
}

void gen_jnz(int label)
{
    gen_emit("or ax, ax");
    gen_emitf("jnz near .L%i", label);
}

// [English] Generates function call and adjusts stack for arguments on 8086
// [Portuguese] Gera chamada de função e ajusta a pilha para argumentos no 8086
void gen_call(const char *name, int nargs)
{
    gen_emitf("call %s", name);
    if (nargs > 0)
    {
        if (nargs == 1)
            gen_emit("pop cx");
        else
            gen_emitf("add sp, %i", nargs * 2);
    }
}

void gen_data_final(void) {}

// [English] 8086 Pattern 1: collapses "mov ax,LABEL; mov bx,ax; mov ax,[bx]" into "mov ax,[LABEL]"
// [Portuguese] Padrão 8086 1: colapsa "mov ax,LABEL; mov bx,ax; mov ax,[bx]" em "mov ax,[LABEL]"
static int i86_match_label_deref(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0;
    if (!peep_op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);
    if (!peep_op_is(&w[2], "mov")) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", basereg);
        if (strcmp(w[2].args[1], expected)) return 0;
    }
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov ax, [%s]", label);
    return n;
}

// [English] 8086 Pattern 2: collapses "lea ax,[bp-N]; mov bx,ax; mov ax,[bx]" into "mov ax,[bp-N]"
// [Portuguese] Padrão 8086 2: colapsa "lea ax,[bp-N]; mov bx,ax; mov ax,[bx]" em "mov ax,[bp-N]"
static int i86_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;
    if (!peep_op_args(&w[1], "mov", 2)) return 0;
    if (strcmp(w[1].args[1], "ax")) return 0;
    char basereg[8];
    strcpy(basereg, w[1].args[0]);
    if (!peep_op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    {
        char expected[32];
        snprintf(expected, sizeof(expected), "[%s]", basereg);
        if (strcmp(w[2].args[1], expected)) return 0;
    }
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov ax, %s", bp_expr);
    return n;
}

// [English] 8086 Pattern 3: collapses "lea ax,[bp-N]; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// into "mov word [bp-N], IMM"
// [Portuguese] Padrão 8086 3: colapsa ... em "mov word [bp-N], IMM"
static int i86_match_lea_push_store(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;
    if (!peep_op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;
    if (!peep_op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    { int dummy; if (!peep_parse_arg_int(val, &dummy)) return 0; }
    if (!peep_op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];
    if (!peep_op_args(&w[4], "mov", 2)) return 0;
    { char expected[32]; snprintf(expected, sizeof(expected), "[%s]", popreg); if (strcmp(w[4].args[0], expected)) return 0; }
    if (strcmp(w[4].args[1], "ax")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov word %s, %s", bp_expr, val);
    return n;
}

// [English] 8086 Pattern 4: replaces "push ax; pop bx" with "mov bx, ax"
// [Portuguese] Padrão 8086 4: substitui "push ax; pop bx" por "mov bx, ax"
static int i86_match_push_pop(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1)) return 0;
    if (!peep_op_args(&w[1], "pop", 1)) return 0;
    if (strcmp(w[0].args[0], w[1].args[0]) == 0) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov %s, %s", w[1].args[0], w[0].args[0]);
    return n;
}

// [English] 8086 Pattern 5: collapses "mov ax,LABEL; push ax; mov ax,IMM; pop bx; mov [bx],ax"
// into "mov word [LABEL], IMM"
// [Portuguese] Padrão 8086 5: colapsa ... em "mov word [LABEL], IMM"
static int i86_match_label_push_store(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *label = w[0].args[1];
    if (label[0] == '[') return 0;
    if (!peep_op_args(&w[1], "push", 1)) return 0;
    if (strcmp(w[1].args[0], "ax")) return 0;
    if (!peep_op_args(&w[2], "mov", 2)) return 0;
    if (strcmp(w[2].args[0], "ax")) return 0;
    const char *val = w[2].args[1];
    { int dummy; if (!peep_parse_arg_int(val, &dummy)) return 0; }
    if (!peep_op_args(&w[3], "pop", 1)) return 0;
    const char *popreg = w[3].args[0];
    if (!peep_op_args(&w[4], "mov", 2)) return 0;
    { char expected[32]; snprintf(expected, sizeof(expected), "[%s]", popreg); if (strcmp(w[4].args[0], expected)) return 0; }
    if (strcmp(w[4].args[1], "ax")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov word [%s], %s", label, val);
    return n;
}

// [English] 8086 Pattern 5: replaces "mov ax, 0" with "xor ax, ax" (shorter, same effect).
// [Portuguese] Padrão 8086 5: substitui "mov ax, 0" por "xor ax, ax" (mais curto, mesmo efeito).
static int i86_match_mov_ax_zero(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "ax") || strcmp(w[0].args[1], "0")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\txor ax, ax");
    return n;
}

// [English] 8086-specific peephole pattern dispatcher.
// [Portuguese] Despachante de padrões peephole específicos 8086.
int gen_peep_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    int n;
    if (wcount == 1) {
        n = i86_match_mov_ax_zero(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 2) {
        n = i86_match_push_pop(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 3) {
        n = i86_match_label_deref(window, repl);
        if (n > 0) return n;
        n = i86_match_lea_deref(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 5) {
        n = i86_match_lea_push_store(window, repl);
        if (n > 0) return n;
        n = i86_match_label_push_store(window, repl);
        if (n > 0) return n;
    }
    return 0;
}
