#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

// [English] Generates a new unique label number for 8085 assembly
// [Portuguese] Gera um novo número de rótulo único para assembly 8085
int gen_label(void) { return label_counter++; }

void gen_text(void) { fprintf(out, "\nsection text\n"); }
void gen_data(void) { fprintf(out, "\nsection data\n"); }
void gen_bss(void) { fprintf(out, "\nsection data\n"); }
void gen_global(const char *name) { fprintf(out, "global %s\n", name); }
void gen_extern(const char *name) { fprintf(out, "extern %s\n", name); }
void gen_label_str(const char *name) { fprintf(out, "%s:\n", name); }
void gen_label_int(int label) { fprintf(out, ".L%i:\n", label); }
void gen_word(int val) { fprintf(out, "\tdw %i\n", val); }
void gen_dword(int val) { fprintf(out, "\tdw %i, 0\n", val & 0xFFFF); }
void gen_reserve(int n) { fprintf(out, "\tds %i\n", n); }

// [English] Emits a string as a DB directive with escape sequence handling for 8085
// [Portuguese] Emite uma string como diretiva DB com tratamento de sequências de escape para 8085
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

// [English] Emits a formatted comment line in 8085 assembly
// [Portuguese] Emite uma linha de comentário formatada em assembly 8085
void gen_comment(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t; ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

// [English] Emits a raw 8085 assembly instruction line
// [Portuguese] Emite uma linha de instrução assembly 8085 bruta
void gen_emit_raw(const char *line)
{
    fprintf(out, "\t%s\n", line);
}

static void gen_emit(const char *line) { fprintf(out, "\t%s\n", line); }

static void gen_emitf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

static int _8080_cur_nlocals = 0;

// [English] Generates function prologue for 8085: saves BC, sets up frame pointer, allocates locals
// [Portuguese] Gera prólogo de função para 8085: salva BC, configura ponteiro de quadro, aloca locais
void gen_prologue(const char *name, int nlocals)
{
    gen_label_str(name);
    _8080_cur_nlocals = nlocals;
    gen_emit("push b");
    gen_emit("lxi h, 0");
    gen_emit("dad sp");
    gen_emit("inx h");
    gen_emit("inx h");
    gen_emit("push h");
    gen_emit("pop b");
    for (int i = 0; i < nlocals; i++) {
        gen_emit("lxi h, 0");
        gen_emit("push h");
    }
}

// [English] Generates function epilogue for 8085: restores SP, pops BC, returns
// [Portuguese] Gera epílogo de função para 8085: restaura SP, desempilha BC, retorna
void gen_epilogue(void)
{
    if (_8080_cur_nlocals > 0) {
        gen_emitf("lxi h, %i", _8080_cur_nlocals * 2);
        gen_emit("dad sp");
        gen_emit("sphl");
    }
    gen_emit("pop b");
    gen_emit("ret");
}

void gen_return(void) { gen_epilogue(); }

void gen_push_prim(void) { gen_emit("push h"); }
void gen_pop_sec(void) { gen_emit("pop d"); }
void gen_load_imm(int val) { gen_emitf("lxi h, %i", val); }
void gen_load_label(int label) { fprintf(outfile, "\tlxi h, .L%i\n", label); }
void gen_load_var(const char *name) { gen_emitf("lhld %s", name); }
void gen_load_addr(const char *name) { gen_emitf("lxi h, %s", name); }

// [English] Stores HL to a local variable at given offset from BC
// [Portuguese] Armazena HL em uma variável local no deslocamento fornecido de BC
void gen_store_local(int offset)
{
    int bo = -(offset * 2 + 4);
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("mov m, a");
    gen_emit("inx h");
    gen_emit("mov m, d");
}

// [English] Loads HL from a local variable at given offset from BC
// [Portuguese] Carrega HL de uma variável local no deslocamento fornecido de BC
void gen_load_local(int offset)
{
    int bo = -(offset * 2 + 4);
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("mov a, m");
    gen_emit("inx h");
    gen_emit("mov h, m");
    gen_emit("mov l, a");
}

// [English] Computes address of a local variable into HL using BC+offset
// [Portuguese] Computa endereço de variável local em HL usando BC+offset
void gen_local_addr(int offset)
{
    gen_emitf("lxi h, %i", -(offset * 2 + 4));
    gen_emit("dad b");
}

// [English] Stores HL to a parameter at given offset from BC
// [Portuguese] Armazena HL em um parâmetro no deslocamento fornecido de BC
void gen_store_param(int offset)
{
    int bo = offset * 2 + 2;
    gen_emit("xchg");
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("mov m, e");
    gen_emit("inx h");
    gen_emit("mov m, d");
}

// [English] Loads HL from a parameter at given offset from BC
// [Portuguese] Carrega HL de um parâmetro no deslocamento fornecido de BC
void gen_load_param(int offset)
{
    int bo = offset * 2 + 2;
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("mov a, m");
    gen_emit("inx h");
    gen_emit("mov h, m");
    gen_emit("mov l, a");
}

// [English] Computes address of a parameter into HL using BC+offset
// [Portuguese] Computa endereço de parâmetro em HL usando BC+offset
void gen_param_addr(int offset)
{
    gen_emitf("lxi h, %i", offset * 2 + 2);
    gen_emit("dad b");
}

void gen_store_global(const char *name) { gen_emitf("shld %s", name); }
void gen_load_global(const char *name) { gen_emitf("lhld %s", name); }

// [English] Dereferences HL: loads the 16-bit value at [HL] into HL
// [Portuguese] Dereferencia HL: carrega o valor de 16 bits em [HL] para HL
void gen_deref(void)
{
    gen_emit("mov a, m");
    gen_emit("inx h");
    gen_emit("mov h, m");
    gen_emit("mov l, a");
}

// [English] Stores DE to the address in HL (16-bit store)
// [Portuguese] Armazena DE no endereço em HL (armazenamento de 16 bits)
void gen_store_to_addr(void)
{
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("mov m, a");
    gen_emit("inx h");
    gen_emit("mov a, d");
    gen_emit("mov m, a");
}

// [English] Reads a byte from [HL] (peekb), zero-extends to 16 bits
// [Portuguese] Lê um byte de [HL] (peekb), estende com zero para 16 bits
void gen_peekb(void)
{
    gen_emit("mov a, m");
    gen_emit("mov l, a");
    gen_emit("mvi h, 0");
}

// [English] Writes a byte (pokeb): stores E to the address in HL
// [Portuguese] Escreve um byte (pokeb): armazena E no endereço em HL
void gen_pokeb(void)
{
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("mov m, a");
}

void gen_add(void) { gen_emit("dad d"); }
void gen_double(void) { gen_emit("dad h"); }

// [English] 16-bit subtraction for 8085: HL = DE - HL
// [Portuguese] Subtração de 16 bits para 8085: HL = DE - HL
void gen_sub(void)
{
    gen_emit("mov a, e");
    gen_emit("sub l");
    gen_emit("mov l, a");
    gen_emit("mov a, d");
    gen_emit("sbb h");
    gen_emit("mov h, a");
}

// [English] 16-bit unsigned multiplication via library call
// [Portuguese] Multiplicação unsigned de 16 bits via chamada de biblioteca
void gen_mul(void)
{
    gen_emit("call __mul16");
}

// [English] 16-bit unsigned division via library call
// [Portuguese] Divisão unsigned de 16 bits via chamada de biblioteca
void gen_div(void)
{
    gen_emit("call __div16");
}

// [English] 16-bit unsigned modulo via library call
// [Portuguese] Módulo unsigned de 16 bits via chamada de biblioteca
void gen_mod(void)
{
    gen_emit("call __mod16");
}

// [English] 16-bit negation (two's complement): HL = -HL
// [Portuguese] Negação de 16 bits (complemento de dois): HL = -HL
void gen_neg(void)
{
    gen_emit("xchg");
    gen_emit("lxi h, 0");
    gen_emit("mov a, l");
    gen_emit("sub e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("sbb d");
    gen_emit("mov h, a");
}

// [English] 16-bit bitwise NOT: HL = ~HL
// [Portuguese] NOT bitwise de 16 bits: HL = ~HL
void gen_not(void)
{
    gen_emit("mov a, l");
    gen_emit("cma");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("cma");
    gen_emit("mov h, a");
}

// [English] 16-bit logical NOT: HL = !HL (returns 0 or 1)
// [Portuguese] NOT lógico de 16 bits: HL = !HL (retorna 0 ou 1)
void gen_lnot(void)
{
    int l_set = gen_label();
    int l_done = gen_label();
    gen_emit("mov a, h");
    gen_emit("ora l");
    gen_emitf("jnz .L%i", l_set);
    gen_emit("lxi h, 1");
    gen_emitf("jmp .L%i", l_done);
    gen_label_int(l_set);
    gen_emit("lxi h, 0");
    gen_label_int(l_done);
}

// [English] 16-bit bitwise AND: HL = HL & DE
// [Portuguese] AND bitwise de 16 bits: HL = HL & DE
void gen_and(void)
{
    gen_emit("mov a, l");
    gen_emit("ana e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("ana d");
    gen_emit("mov h, a");
}

// [English] 16-bit bitwise OR: HL = HL | DE
// [Portuguese] OR bitwise de 16 bits: HL = HL | DE
void gen_or(void)
{
    gen_emit("mov a, l");
    gen_emit("ora e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("ora d");
    gen_emit("mov h, a");
}

// [English] 16-bit bitwise XOR: HL = HL ^ DE
// [Portuguese] XOR bitwise de 16 bits: HL = HL ^ DE
void gen_xor(void)
{
    gen_emit("mov a, l");
    gen_emit("xra e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("xra d");
    gen_emit("mov h, a");
}

// [English] 16-bit left shift: HL = DE << HL
// [Portuguese] Deslocamento à esquerda de 16 bits: HL = DE << HL
void gen_shl(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("ora a");
    gen_emitf("jz .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("dad h");
    gen_emit("dcr a");
    gen_emitf("jnz .L%i", l_loop);
    gen_label_int(l_done);
}

// [English] 16-bit right shift for 8085: HL = DE >> HL (uses RAR with push/pop PSW)
// [Portuguese] Deslocamento à direita de 16 bits para 8085: HL = DE >> HL (usa RAR com push/pop PSW)
void gen_shr(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("ora a");
    gen_emitf("jz .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("push psw");
    gen_emit("mov a, h");
    gen_emit("rar");
    gen_emit("mov h, a");
    gen_emit("mov a, l");
    gen_emit("rar");
    gen_emit("mov l, a");
    gen_emit("pop psw");
    gen_emit("dcr a");
    gen_emitf("jnz .L%i", l_loop);
    gen_label_int(l_done);
}

#define GEN_CMPCODE \
    gen_emit("mov a, l"); \
    gen_emit("sub e"); \
    gen_emit("mov l, a"); \
    gen_emit("mov a, h"); \
    gen_emit("sbb d"); \
    gen_emit("mov h, a")

// [English] 16-bit equality comparison: HL = (HL == DE) ? 1 : 0
// [Portuguese] Comparação de igualdade de 16 bits: HL = (HL == DE) ? 1 : 0
void gen_cmp_eq(void) { int l0=gen_label(),lm=gen_label(); gen_emit("mov a,l"); gen_emit("sub e"); gen_emitf("jnz .L%i",l0); gen_emit("mov a,h"); gen_emit("sbb d"); gen_emitf("jnz .L%i",l0); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(l0); gen_emit("lxi h,0"); gen_label_int(lm); }
// [English] 16-bit not-equal comparison: HL = (HL != DE) ? 1 : 0
// [Portuguese] Comparação de desigualdade de 16 bits: HL = (HL != DE) ? 1 : 0
void gen_cmp_ne(void) { int l0=gen_label(),lm=gen_label(); gen_emit("mov a,l"); gen_emit("sub e"); gen_emitf("jnz .L%i",l0); gen_emit("mov a,h"); gen_emit("sbb d"); gen_emit("lxi h,1"); gen_emitf("jnz .L%i",lm); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(l0); gen_emit("lxi h,1"); gen_label_int(lm); }
// [English] 16-bit less-than comparison: HL = (DE < HL) ? 1 : 0
// [Portuguese] Comparação menor-que de 16 bits: HL = (DE < HL) ? 1 : 0
void gen_cmp_lt(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,1"); gen_label_int(lm); }
// [English] 16-bit greater-than comparison: HL = (DE > HL) ? 1 : 0
// [Portuguese] Comparação maior-que de 16 bits: HL = (DE > HL) ? 1 : 0
void gen_cmp_gt(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emitf("jz .L%i",lt); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,0"); gen_label_int(lm); }
// [English] 16-bit less-or-equal comparison: HL = (DE <= HL) ? 1 : 0
// [Portuguese] Comparação menor-ou-igual de 16 bits: HL = (DE <= HL) ? 1 : 0
void gen_cmp_le(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emitf("jz .L%i",lt); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,1"); gen_label_int(lm); }
// [English] 16-bit greater-or-equal comparison: HL = (DE >= HL) ? 1 : 0
// [Portuguese] Comparação maior-ou-igual de 16 bits: HL = (DE >= HL) ? 1 : 0
void gen_cmp_ge(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,0"); gen_label_int(lm); }

void gen_jmp(int label) { gen_emitf("jmp .L%i", label); }
void gen_jz(int label) { gen_emit("mov a, h"); gen_emit("ora l"); gen_emitf("jz .L%i", label); }
void gen_jnz(int label) { gen_emit("mov a, h"); gen_emit("ora l"); gen_emitf("jnz .L%i", label); }

// [English] Generates function call and adjusts stack for arguments on 8085
// [Portuguese] Gera chamada de função e ajusta a pilha para argumentos no 8085
void gen_call(const char *name, int nargs)
{
    gen_emitf("call %s", name);
    if (nargs > 0) {
        gen_emit("xchg");
        gen_emitf("lxi h, %i", nargs * 2);
        gen_emit("dad sp");
        gen_emit("sphl");
        gen_emit("xchg");
    }
}

void gen_data_final(void) {}

// [English] 8085 peephole pattern dispatcher (identical to 8080).
// [Portuguese] Despachante de padrões peephole 8085 (idêntico ao 8080).
int gen_peep_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    (void)window; (void)wcount; (void)repl;
    return 0;
}
