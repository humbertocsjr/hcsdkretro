#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;
static int emit_mul = 1;
static int emit_div = 1;

// [English] Generates a new unique label number for Z80 assembly
// [Portuguese] Gera um novo número de rótulo único para assembly Z80
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

// [English] Emits a string as a DB (define byte) directive with escape sequence handling
// [Portuguese] Emite uma string como diretiva DB (define byte) com tratamento de sequências de escape
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

// [English] Emits a formatted comment line in Z80 assembly
// [Portuguese] Emite uma linha de comentário formatada em assembly Z80
void gen_comment(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t; ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

// [English] Emits a raw assembly instruction line prefixed with a tab
// [Portuguese] Emite uma linha de instrução assembly bruta prefixada com tabulação
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

// [English] Generates function prologue: saves IX, sets up IX=SP, allocates locals on stack
// [Portuguese] Gera prólogo de função: salva IX, configura IX=SP, aloca locais na pilha
void gen_prologue(const char *name, int nlocals)
{
    gen_label_str(name);
    gen_emit("push ix");
    gen_emit("ld ix, 0");
    gen_emit("add ix, sp");
    if (nlocals > 0)
    {
        if (nlocals <= 2)
        {
            for (int i = 0; i < nlocals * 2; i++)
                gen_emit("dec sp");
        }
        else
        {
            gen_emitf("ld hl, -%i", nlocals * 2);
            gen_emit("add hl, sp");
            gen_emit("ld sp, hl");
        }
    }
}

// [English] Generates function epilogue: restores SP from IX, pops IX, returns
// [Portuguese] Gera epílogo de função: restaura SP de IX, desempilha IX, retorna
void gen_epilogue(void)
{
    gen_emit("ld sp, ix");
    gen_emit("pop ix");
    gen_emit("ret");
}

// [English] Generates a return (same as epilogue for Z80)
// [Portuguese] Gera um retorno (mesmo que epílogo para Z80)
void gen_return(void)
{
    gen_epilogue();
}

// [English] Pushes the primary register (HL) onto the stack
// [Portuguese] Empilha o registrador primário (HL) na pilha
void gen_push_prim(void)
{
    gen_emit("push hl");
}

// [English] Pops the secondary register (DE) from the stack
// [Portuguese] Desempilha o registrador secundário (DE) da pilha
void gen_pop_sec(void)
{
    gen_emit("pop de");
}

// [English] Exchanges primary (HL) and secondary (DE) registers
// [Portuguese] Troca os registradores primário (HL) e secundário (DE)
void gen_exchange(void)
{
    gen_emit("ex de, hl");
}

// [English] Loads an immediate 16-bit value into HL
// [Portuguese] Carrega um valor imediato de 16 bits em HL
void gen_load_imm(int val)
{
    gen_emitf("ld hl, %i", val);
}

// [English] Loads HL with the 16-bit value stored at the named variable's address
// [Portuguese] Carrega HL com o valor de 16 bits armazenado no endereço da variável nomeada
void gen_load_var(const char *name)
{
    gen_emitf("ld hl, [%s]", name);
}

// [English] Loads HL with the address of a named variable
// [Portuguese] Carrega HL com o endereço de uma variável nomeada
void gen_load_addr(const char *name)
{
    gen_emitf("ld hl, %s", name);
}

// [English] Loads HL with the address of a compiler-generated label
// [Portuguese] Carrega HL com o endereço de um rótulo gerado pelo compilador
void gen_load_label(int label) { fprintf(outfile, "\tld hl, .L%i\n", label); }

// [English] Stores the 16-bit value in HL to a named global variable
// [Portuguese] Armazena o valor de 16 bits em HL em uma variável global nomeada
void gen_store_global(const char *name)
{
    gen_emitf("ld [%s], hl", name);
}

// [English] Loads HL with the 16-bit value from a named global variable
// [Portuguese] Carrega HL com o valor de 16 bits de uma variável global nomeada
void gen_load_global(const char *name)
{
    gen_emitf("ld hl, [%s]", name);
}

// [English] Stores HL to a local variable at given offset from IX
// [Portuguese] Armazena HL em uma variável local no deslocamento fornecido de IX
void gen_store_local(int offset)
{
    gen_emitf("ld [ix+%i], l", -(offset * 2 + 2));
    gen_emitf("ld [ix+%i], h", -(offset * 2 + 1));
}

// [English] Loads HL from a local variable at given offset from IX
// [Portuguese] Carrega HL de uma variável local no deslocamento fornecido de IX
void gen_load_local(int offset)
{
    gen_emitf("ld l, [ix+%i]", -(offset * 2 + 2));
    gen_emitf("ld h, [ix+%i]", -(offset * 2 + 1));
}

// [English] Computes the address of a local variable into HL using IX+offset
// [Portuguese] Computa o endereço de uma variável local em HL usando IX+offset
void gen_local_addr(int offset)
{
    gen_emit("push ix");
    gen_emit("pop de");
    gen_emitf("ld hl, %i", -(offset * 2 + 2));
    gen_emit("add hl, de");
}

// [English] Stores HL to a parameter at given offset from IX
// [Portuguese] Armazena HL em um parâmetro no deslocamento fornecido de IX
void gen_store_param(int offset)
{
    gen_emitf("ld [ix+%i], l", offset * 2 + 4);
    gen_emitf("ld [ix+%i], h", offset * 2 + 5);
}

// [English] Loads HL from a parameter at given offset from IX
// [Portuguese] Carrega HL de um parâmetro no deslocamento fornecido de IX
void gen_load_param(int offset)
{
    gen_emitf("ld l, [ix+%i]", offset * 2 + 4);
    gen_emitf("ld h, [ix+%i]", offset * 2 + 5);
}

// [English] Computes the address of a parameter into HL using IX+offset
// [Portuguese] Computa o endereço de um parâmetro em HL usando IX+offset
void gen_param_addr(int offset)
{
    gen_emit("push ix");
    gen_emit("pop de");
    gen_emitf("ld hl, %i", offset * 2 + 4);
    gen_emit("add hl, de");
}

// [English] Dereferences HL: loads the 16-bit value at address HL into HL
// [Portuguese] Dereferencia HL: carrega o valor de 16 bits no endereço HL em HL
void gen_deref(void)
{
    gen_emit("ld a, [hl]");
    gen_emit("inc hl");
    gen_emit("ld h, [hl]");
    gen_emit("ld l, a");
}

// [English] Reads a byte from the address in HL (peekb), zero-extends to 16 bits
// [Portuguese] Lê um byte do endereço em HL (peekb), estende com zero para 16 bits
void gen_peekb(void)
{
    gen_emit("ld l, [hl]");
    gen_emit("ld h, 0");
}

// [English] Writes a byte (pokeb): stores E to the address in HL
// [Portuguese] Escreve um byte (pokeb): armazena E no endereço em HL
void gen_pokeb(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld [hl], e");
}

// [English] Stores the 16-bit value in DE to the address in HL (word poke)
// [Portuguese] Armazena o valor de 16 bits em DE no endereço em HL (poke word)
void gen_store_to_addr(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld [hl], e");
    gen_emit("inc hl");
    gen_emit("ld [hl], d");
}

// [English] 16-bit addition: HL = HL + DE
// [Portuguese] Adição de 16 bits: HL = HL + DE
void gen_add(void)
{
    gen_emit("add hl, de");
}

// [English] Doubles the 16-bit value in HL: HL = HL * 2
// [Portuguese] Dobra o valor de 16 bits em HL: HL = HL * 2
void gen_double(void)
{
    gen_emit("add hl, hl");
}

// [English] 16-bit subtraction: HL = DE - HL (performs HL = 0 - HL after ex de,hl)
// [Portuguese] Subtração de 16 bits: HL = DE - HL (faz HL = 0 - HL após ex de,hl)
void gen_sub(void)
{
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
}

// [English] 16-bit unsigned multiplication using shift-and-add algorithm
// [Portuguese] Multiplicação unsigned de 16 bits usando algoritmo shift-and-add
void gen_mul(void)
{
    int l = gen_label();
    int l_skip = gen_label();
    gen_emit("push de");
    gen_emit("push hl");
    gen_emit("pop bc");
    gen_emit("ld hl, 0");
    gen_emitf("ld a, %i", 16);
    gen_label_int(l);
    gen_emit("srl b");
    gen_emit("rr c");
    gen_emitf("jr nc, .L%i", l_skip);
    gen_emit("add hl, de");
    gen_label_int(l_skip);
    gen_emit("sla e");
    gen_emit("rl d");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l);
    gen_emit("pop de");
}

// [English] 16-bit unsigned division using shift-and-subtract algorithm
// [Portuguese] Divisão unsigned de 16 bits usando algoritmo shift-and-subtract
void gen_div(void)
{
    int l = gen_label();
    int l_sub = gen_label();
    int l_done = gen_label();
    gen_emit("ex de, hl");
    gen_emit("ld a, 16");
    gen_emit("ld bc, 0");
    gen_label_int(l);
    gen_emit("push af");
    gen_emit("add hl, hl");
    gen_emit("rl c");
    gen_emit("rl b");
    gen_emit("ld a, c");
    gen_emit("sub e");
    gen_emit("ld c, a");
    gen_emit("ld a, b");
    gen_emit("sbc a, d");
    gen_emit("ld b, a");
    gen_emitf("jr c, .L%i", l_sub);
    gen_emit("inc hl");
    gen_emitf("jr .L%i", l_done);
    gen_label_int(l_sub);
    gen_emit("ld a, c");
    gen_emit("add a, e");
    gen_emit("ld c, a");
    gen_emit("ld a, b");
    gen_emit("adc a, d");
    gen_emit("ld b, a");
    gen_label_int(l_done);
    gen_emit("pop af");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l);
}

// [English] 16-bit unsigned modulo: same as division but returns remainder in BC
// [Portuguese] Módulo unsigned de 16 bits: mesma lógica da divisão mas retorna resto em BC
void gen_mod(void)
{
    int l = gen_label();
    int l_sub = gen_label();
    int l_done = gen_label();
    gen_emit("ex de, hl");
    gen_emit("ld a, 16");
    gen_emit("ld bc, 0");
    gen_label_int(l);
    gen_emit("push af");
    gen_emit("add hl, hl");
    gen_emit("rl c");
    gen_emit("rl b");
    gen_emit("ld a, c");
    gen_emit("sub e");
    gen_emit("ld c, a");
    gen_emit("ld a, b");
    gen_emit("sbc a, d");
    gen_emit("ld b, a");
    gen_emitf("jr c, .L%i", l_sub);
    gen_emit("inc hl");
    gen_emitf("jr .L%i", l_done);
    gen_label_int(l_sub);
    gen_emit("ld a, c");
    gen_emit("add a, e");
    gen_emit("ld c, a");
    gen_emit("ld a, b");
    gen_emit("adc a, d");
    gen_emit("ld b, a");
    gen_label_int(l_done);
    gen_emit("pop af");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l);
    gen_emit("push bc");
    gen_emit("pop hl");
}

// [English] 16-bit negation (two's complement): HL = -HL
// [Portuguese] Negação de 16 bits (complemento de dois): HL = -HL
void gen_neg(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld hl, 0");
    gen_emit("or a");
    gen_emit("sbc hl, de");
}

// [English] 16-bit bitwise NOT: HL = ~HL
// [Portuguese] NOT bitwise de 16 bits: HL = ~HL
void gen_not(void)
{
    gen_emit("ld a, l");
    gen_emit("cpl");
    gen_emit("ld l, a");
    gen_emit("ld a, h");
    gen_emit("cpl");
    gen_emit("ld h, a");
}

// [English] 16-bit logical NOT: HL = !HL (returns 0 or 1)
// [Portuguese] NOT lógico de 16 bits: HL = !HL (retorna 0 ou 1)
void gen_lnot(void)
{
    int l_set = gen_label();
    int l_done = gen_label();
    gen_emit("ld a, h");
    gen_emit("or l");
    gen_emitf("jr nz, .L%i", l_set);
    gen_emit("ld hl, 1");
    gen_emitf("jr .L%i", l_done);
    gen_label_int(l_set);
    gen_emit("ld hl, 0");
    gen_label_int(l_done);
}

// [English] 16-bit bitwise AND: HL = HL & DE
// [Portuguese] AND bitwise de 16 bits: HL = HL & DE
void gen_and(void)
{
    gen_emit("ld a, e");
    gen_emit("and l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("and h");
    gen_emit("ld h, a");
}

// [English] 16-bit bitwise OR: HL = HL | DE
// [Portuguese] OR bitwise de 16 bits: HL = HL | DE
void gen_or(void)
{
    gen_emit("ld a, e");
    gen_emit("or l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("or h");
    gen_emit("ld h, a");
}

// [English] 16-bit bitwise XOR: HL = HL ^ DE
// [Portuguese] XOR bitwise de 16 bits: HL = HL ^ DE
void gen_xor(void)
{
    gen_emit("ld a, e");
    gen_emit("xor l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("xor h");
    gen_emit("ld h, a");
}

// [English] 16-bit left shift: HL = DE << HL (shift count in HL, value in DE)
// [Portuguese] Deslocamento à esquerda de 16 bits: HL = DE << HL (contagem em HL, valor em DE)
void gen_shl(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    gen_emit("ex de, hl");
    gen_emit("ld a, e");
    gen_emit("or a");
    gen_emitf("jr z, .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("add hl, hl");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l_loop);
    gen_label_int(l_done);
}

// [English] 16-bit right shift: HL = DE >> HL (shift count in HL, value in DE)
// [Portuguese] Deslocamento à direita de 16 bits: HL = DE >> HL (contagem em HL, valor em DE)
void gen_shr(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    gen_emit("ex de, hl");
    gen_emit("ld a, e");
    gen_emit("or a");
    gen_emitf("jr z, .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("srl h");
    gen_emit("rr l");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l_loop);
    gen_label_int(l_done);
}

// [English] 16-bit equality comparison: HL = (HL == DE) ? 1 : 0
// [Portuguese] Comparação de igualdade de 16 bits: HL = (HL == DE) ? 1 : 0
void gen_cmp_eq(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr nz, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] 16-bit not-equal comparison: HL = (HL != DE) ? 1 : 0
// [Portuguese] Comparação de desigualdade de 16 bits: HL = (HL != DE) ? 1 : 0
void gen_cmp_ne(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr z, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] 16-bit less-than comparison: HL = (DE < HL) ? 1 : 0
// [Portuguese] Comparação menor-que de 16 bits: HL = (DE < HL) ? 1 : 0
void gen_cmp_lt(void)
{
    int l1 = gen_label();
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr nc, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] 16-bit greater-than comparison: HL = (DE > HL) ? 1 : 0
// [Portuguese] Comparação maior-que de 16 bits: HL = (DE > HL) ? 1 : 0
void gen_cmp_gt(void)
{
    int l1 = gen_label();
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr z, .L%i", l1);
    gen_emitf("jr c, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] 16-bit less-or-equal comparison: HL = (DE <= HL) ? 1 : 0
// [Portuguese] Comparação menor-ou-igual de 16 bits: HL = (DE <= HL) ? 1 : 0
void gen_cmp_le(void)
{
    int l1 = gen_label();
    int l2 = gen_label();
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr c, .L%i", l2);
    gen_emitf("jr z, .L%i", l2);
    gen_emitf("jr .L%i", l1);
    gen_label_int(l2);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] 16-bit greater-or-equal comparison: HL = (DE >= HL) ? 1 : 0
// [Portuguese] Comparação maior-ou-igual de 16 bits: HL = (DE >= HL) ? 1 : 0
void gen_cmp_ge(void)
{
    int l1 = gen_label();
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr c, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

// [English] Unconditional jump to a compiler-generated label
// [Portuguese] Salto incondicional para um rótulo gerado pelo compilador
void gen_jmp(int label)
{
    gen_emitf("jp .L%i", label);
}

// [English] Conditional jump if zero: jumps if HL == 0
// [Portuguese] Salto condicional se zero: salta se HL == 0
void gen_jz(int label)
{
    gen_emit("ld a, h");
    gen_emit("or l");
    gen_emitf("jp z, .L%i", label);
}

// [English] Conditional jump if non-zero: jumps if HL != 0
// [Portuguese] Salto condicional se não-zero: salta se HL != 0
void gen_jnz(int label)
{
    gen_emit("ld a, h");
    gen_emit("or l");
    gen_emitf("jp nz, .L%i", label);
}

// [English] Generates a function call and adjusts stack for arguments
// [Portuguese] Gera uma chamada de função e ajusta a pilha para argumentos
void gen_call(const char *name, int nargs)
{
    gen_emitf("call %s", name);
    if (nargs > 0)
    {
        gen_emit("ex de, hl");
        gen_emitf("ld hl, %i", nargs * 2);
        gen_emit("add hl, sp");
        gen_emit("ld sp, hl");
        gen_emit("ex de, hl");
    }
}

void gen_data_final(void) {}

// [English] Z80 Pattern 1: optimizes local variable load with computed offset into direct IX load.
// Replaces 8 instructions with 2 direct IX loads: ld l,[ix+N]; ld h,[ix+N+1]
// [Portuguese] Padrão Z80 1: otimiza carga de variável local com deslocamento computado para carga direta por IX.
static int z80_match_local_load(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1) || strcmp(w[0].args[0], "ix")) return 0;
    if (!peep_op_args(&w[1], "pop", 1) || strcmp(w[1].args[0], "de")) return 0;
    if (w[2].nargs != 2) return 0;
    if (!peep_op_is(&w[2], "ld") || strcmp(w[2].args[0], "hl")) return 0;
    int off;
    if (!peep_parse_arg_int(w[2].args[1], &off)) return 0;
    if (!peep_op_is(&w[3], "add") || strcmp(w[3].args[0], "hl") || strcmp(w[3].args[1], "de")) return 0;
    if (!peep_op_is(&w[4], "ld") || strcmp(w[4].args[0], "a") || strcmp(w[4].args[1], "[hl]")) return 0;
    if (!peep_op_is(&w[5], "inc") || strcmp(w[5].args[0], "hl")) return 0;
    if (!peep_op_is(&w[6], "ld") || strcmp(w[6].args[0], "h") || strcmp(w[6].args[1], "[hl]")) return 0;
    if (!peep_op_is(&w[7], "ld") || strcmp(w[7].args[0], "l") || strcmp(w[7].args[1], "a")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld l, [ix%+d]", off);
    peep_emit_repl(repl, &n, "\tld h, [ix%+d]", off + 1);
    return n;
}

// [English] Z80 Pattern 2: alternate register order for local variable load (uses e/d instead of a).
// [Portuguese] Padrão Z80 2: ordem alternativa de registradores para carga de variável local (usa e/d em vez de a).
static int z80_match_local_load2(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1) || strcmp(w[0].args[0], "ix")) return 0;
    if (!peep_op_args(&w[1], "pop", 1) || strcmp(w[1].args[0], "de")) return 0;
    if (w[2].nargs != 2) return 0;
    if (!peep_op_is(&w[2], "ld") || strcmp(w[2].args[0], "hl")) return 0;
    int off;
    if (!peep_parse_arg_int(w[2].args[1], &off)) return 0;
    if (!peep_op_is(&w[3], "add") || strcmp(w[3].args[0], "hl") || strcmp(w[3].args[1], "de")) return 0;
    if (!peep_op_is(&w[4], "ld") || strcmp(w[4].args[0], "e") || strcmp(w[4].args[1], "[hl]")) return 0;
    if (!peep_op_is(&w[5], "inc") || strcmp(w[5].args[0], "hl")) return 0;
    if (!peep_op_is(&w[6], "ld") || strcmp(w[6].args[0], "d") || strcmp(w[6].args[1], "[hl]")) return 0;
    if (!peep_op_is(&w[7], "ex") || strcmp(w[7].args[0], "de") || strcmp(w[7].args[1], "hl")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld l, [ix%+d]", off);
    peep_emit_repl(repl, &n, "\tld h, [ix%+d]", off + 1);
    return n;
}

// [English] Z80 Pattern 3: optimizes storing an immediate value to a local variable.
// Replaces 12 instructions with 3: ld hl,IMM; ld [ix+N],l; ld [ix+N+1],h
// [Portuguese] Padrão Z80 3: otimiza armazenamento de valor imediato em variável local.
static int z80_match_local_store_imm(peep_line_t *w, peep_line_t *repl)
{
    if (w[0].nargs != 2) return 0;
    if (!peep_op_is(&w[0], "ld") || strcmp(w[0].args[0], "hl")) return 0;
    int off;
    if (!peep_parse_arg_int(w[0].args[1], &off)) return 0;
    if (!peep_op_is(&w[1], "ex") || strcmp(w[1].args[0], "de") || strcmp(w[1].args[1], "hl")) return 0;
    if (!peep_op_is(&w[2], "push") || strcmp(w[2].args[0], "ix")) return 0;
    if (!peep_op_is(&w[3], "pop") || strcmp(w[3].args[0], "hl")) return 0;
    if (!peep_op_is(&w[4], "add") || strcmp(w[4].args[0], "hl") || strcmp(w[4].args[1], "de")) return 0;
    if (!peep_op_is(&w[5], "push") || strcmp(w[5].args[0], "hl")) return 0;
    if (!peep_op_is(&w[6], "ld") || strcmp(w[6].args[0], "hl")) return 0;
    int imm;
    if (!peep_parse_arg_int(w[6].args[1], &imm)) return 0;
    if (!peep_op_is(&w[7], "pop") || strcmp(w[7].args[0], "de")) return 0;
    if (!peep_op_is(&w[8], "ex") || strcmp(w[8].args[0], "de") || strcmp(w[8].args[1], "hl")) return 0;
    if (!peep_op_is(&w[9], "ld") || strcmp(w[9].args[0], "[hl]") || strcmp(w[9].args[1], "e")) return 0;
    if (!peep_op_is(&w[10], "inc") || strcmp(w[10].args[0], "hl")) return 0;
    if (!peep_op_is(&w[11], "ld") || strcmp(w[11].args[0], "[hl]") || strcmp(w[11].args[1], "d")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld hl, %d", imm);
    peep_emit_repl(repl, &n, "\tld [ix%+d], l", off);
    peep_emit_repl(repl, &n, "\tld [ix%+d], h", off + 1);
    return n;
}

// [English] Z80 Pattern 4: replaces "ld hl,N; add hl,sp; ld sp,hl" with N×inc sp for small N.
// Replaces 3 instructions (7 bytes) with N inc sp instructions (N bytes) for N ≤ 6.
// [Portuguese] Padrão Z80 4: substitui "ld hl,N; add hl,sp; ld sp,hl" por N×inc sp para N pequeno.
static int z80_match_inc_sp(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "ld", 2)) return 0;
    if (strcmp(w[0].args[0], "hl")) return 0;
    if (!peep_op_is(&w[1], "add") || w[1].nargs != 2) return 0;
    if (strcmp(w[1].args[0], "hl") || strcmp(w[1].args[1], "sp")) return 0;
    if (!peep_op_args(&w[2], "ld", 2)) return 0;
    if (strcmp(w[2].args[0], "sp") || strcmp(w[2].args[1], "hl")) return 0;
    int val;
    if (!peep_parse_arg_int(w[0].args[1], &val)) return 0;
    if (val <= 0 || val > 6) return 0;
    int n = 0;
    for (int i = 0; i < val; i++)
        peep_emit_repl(repl, &n, "\tinc sp");
    return n;
}

// [English] Z80 Pattern 5: replaces "push bc; pop hl" with "ld h,b; ld l,c" (same size, faster).
// [Portuguese] Padrão Z80 5: substitui "push bc; pop hl" por "ld h,b; ld l,c" (mesmo tamanho, mais rápido).
static int z80_match_pushbc_pophl(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1)) return 0;
    if (!peep_op_args(&w[1], "pop", 1)) return 0;
    if (strcmp(w[0].args[0], "bc") || strcmp(w[1].args[0], "hl")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld h, b");
    peep_emit_repl(repl, &n, "\tld l, c");
    return n;
}

// [English] Z80 Pattern 6: removes redundant "ex de,hl" around "inc sp" sequences.
// Pattern: ex de,hl; inc sp; ...; inc sp; ex de,hl  →  inc sp; ...; inc sp
// [Portuguese] Padrão Z80 6: remove "ex de,hl" redundante ao redor de sequências "inc sp".
static int z80_match_ex_sp(peep_line_t *w, int wcount, peep_line_t *repl)
{
    if (wcount < 4) return 0;
    if (!peep_op_is(&w[0], "ex") || w[0].nargs != 2) return 0;
    if (strcmp(w[0].args[0], "de") || strcmp(w[0].args[1], "hl")) return 0;
    // Middle instructions must all be "inc sp"
    for (int k = 1; k < wcount - 1; k++) {
        if (!peep_op_args(&w[k], "inc", 1)) return 0;
        if (strcmp(w[k].args[0], "sp")) return 0;
    }
    if (!peep_op_is(&w[wcount-1], "ex") || w[wcount-1].nargs != 2) return 0;
    if (strcmp(w[wcount-1].args[0], "de") || strcmp(w[wcount-1].args[1], "hl")) return 0;
    int n = 0;
    for (int k = 1; k < wcount - 1; k++)
        peep_emit_repl(repl, &n, "\tinc sp");
    return n;
}

// [English] Z80 Pattern 7: replaces "ex de,hl; ld hl,N; add hl,sp; ld sp,hl; ex de,hl" with N×inc sp (N even, ≤6).
// Eliminates the 5-instruction call argument cleanup for small call footprints.
// [Portuguese] Padrão Z80 7: substitui sequência de limpeza de argumentos de chamada por N×inc sp.
static int z80_match_call_cleanup(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_is(&w[0], "ex") || w[0].nargs != 2) return 0;
    if (strcmp(w[0].args[0], "de") || strcmp(w[0].args[1], "hl")) return 0;
    if (!peep_op_args(&w[1], "ld", 2)) return 0;
    if (strcmp(w[1].args[0], "hl")) return 0;
    if (!peep_op_is(&w[2], "add") || w[2].nargs != 2) return 0;
    if (strcmp(w[2].args[0], "hl") || strcmp(w[2].args[1], "sp")) return 0;
    if (!peep_op_args(&w[3], "ld", 2)) return 0;
    if (strcmp(w[3].args[0], "sp") || strcmp(w[3].args[1], "hl")) return 0;
    if (!peep_op_is(&w[4], "ex") || w[4].nargs != 2) return 0;
    if (strcmp(w[4].args[0], "de") || strcmp(w[4].args[1], "hl")) return 0;
    int val;
    if (!peep_parse_arg_int(w[1].args[1], &val)) return 0;
    if (val <= 0 || val > 6 || (val & 1)) return 0;
    int n = 0;
    for (int i = 0; i < val; i++)
        peep_emit_repl(repl, &n, "\tinc sp");
    return n;
}

// [English] Z80 Pattern 8: replaces "push hl; ld hl,IMM; pop de; ex de,hl" with "ld de,IMM".
// Eliminates redundant push/pop when loading an immediate into DE for comparisons/subtractions.
// [Portuguese] Padrão Z80 8: substitui push hl/immediate/pop de/ex de,hl por ld de,IMM.
static int z80_match_push_imm_pop_ex(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1) || strcmp(w[0].args[0], "hl")) return 0;
    if (!peep_op_args(&w[1], "ld", 2) || strcmp(w[1].args[0], "hl")) return 0;
    int imm;
    if (!peep_parse_arg_int(w[1].args[1], &imm)) return 0;
    if (!peep_op_args(&w[2], "pop", 1) || strcmp(w[2].args[0], "de")) return 0;
    if (!peep_op_is(&w[3], "ex") || w[3].nargs != 2) return 0;
    if (strcmp(w[3].args[0], "de") || strcmp(w[3].args[1], "hl")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld de, %d", imm);
    return n;
}

// [English] Z80 Pattern 9: replaces "push hl; ld hl,IMM; pop de; add hl,de" with "ld de,IMM; add hl,de".
// Same optimization as Pattern 8 but for addition operations.
// [Portuguese] Padrão Z80 9: mesma otimização do Padrão 8 mas para operações de adição.
static int z80_match_push_imm_pop_add(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1) || strcmp(w[0].args[0], "hl")) return 0;
    if (!peep_op_args(&w[1], "ld", 2) || strcmp(w[1].args[0], "hl")) return 0;
    int imm;
    if (!peep_parse_arg_int(w[1].args[1], &imm)) return 0;
    if (!peep_op_args(&w[2], "pop", 1) || strcmp(w[2].args[0], "de")) return 0;
    if (!peep_op_is(&w[3], "add") || w[3].nargs != 2) return 0;
    if (strcmp(w[3].args[0], "hl") || strcmp(w[3].args[1], "de")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld de, %d", imm);
    peep_emit_repl(repl, &n, "\tadd hl, de");
    return n;
}

// [English] Z80 Pattern 10: replaces "push hl; ld l,[ix+N]; ld h,[ix+N+1]; pop de; ex de,hl"
// with "ld e,[ix+N]; ld d,[ix+N+1]" when a simple variable is the right operand.
// [Portuguese] Padrão Z80 10: substitui push hl/carga de variável local/pop de/ex de,hl
// por carga direta em DE quando variável simples é o operando direito.
static int z80_match_push_var_pop_ex(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1) || strcmp(w[0].args[0], "hl")) return 0;
    if (!peep_op_args(&w[1], "ld", 2) || strcmp(w[1].args[0], "l")) return 0;
    const char *a1 = w[1].args[1];
    if (a1[0] != '[' || a1[1] != 'i' || a1[2] != 'x') return 0;
    int off1 = (int)strtol(a1 + 3, NULL, 10);
    if (off1 == 0 && a1[3] != '0') return 0;
    if (!peep_op_args(&w[2], "ld", 2) || strcmp(w[2].args[0], "h")) return 0;
    const char *a2 = w[2].args[1];
    if (a2[0] != '[' || a2[1] != 'i' || a2[2] != 'x') return 0;
    int off2 = (int)strtol(a2 + 3, NULL, 10);
    if (off2 == 0 && a2[3] != '0') return 0;
    if (off1 != off2 - 1) return 0;
    if (!peep_op_args(&w[3], "pop", 1) || strcmp(w[3].args[0], "de")) return 0;
    if (!peep_op_is(&w[4], "ex") || w[4].nargs != 2) return 0;
    if (strcmp(w[4].args[0], "de") || strcmp(w[4].args[1], "hl")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tld e, [ix%+d]", off1);
    peep_emit_repl(repl, &n, "\tld d, [ix%+d]", off2);
    return n;
}

// [English] Z80-specific peephole pattern dispatcher.
// [Portuguese] Despachante de padrões peephole específicos Z80.
int gen_peep_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    if (wcount >= 4) {
        int n = z80_match_ex_sp(window, wcount, repl);
        if (n > 0) return n;
    }
    if (wcount == 4) {
        int n = z80_match_push_imm_pop_ex(window, repl);
        if (n > 0) return n;
        n = z80_match_push_imm_pop_add(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 8) {
        int n = z80_match_local_load(window, repl);
        if (n > 0) return n;
        n = z80_match_local_load2(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 12) {
        int n = z80_match_local_store_imm(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 5) {
        int n = z80_match_call_cleanup(window, repl);
        if (n > 0) return n;
        n = z80_match_push_var_pop_ex(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 3) {
        int n = z80_match_inc_sp(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 2) {
        int n = z80_match_pushbc_pophl(window, repl);
        if (n > 0) return n;
    }
    return 0;
}
