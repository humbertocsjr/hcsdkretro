#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

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

void gen_comment(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(out, "\t; ");
    vfprintf(out, fmt, args);
    fprintf(out, "\n");
    va_end(args);
}

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

// Locals at BC - 2 - nlocals*2 + offset*2
void gen_store_local(int offset)
{
    int bo = offset * 2 - 2 - _8080_cur_nlocals * 2;
    gen_emit("xchg");
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("mov m, e");
    gen_emit("inx h");
    gen_emit("mov m, d");
}

void gen_load_local(int offset)
{
    int bo = offset * 2 - 2 - _8080_cur_nlocals * 2;
    gen_emitf("lxi h, %i", bo);
    gen_emit("dad b");
    gen_emit("mov a, m");
    gen_emit("inx h");
    gen_emit("mov h, m");
    gen_emit("mov l, a");
}

void gen_local_addr(int offset)
{
    gen_emitf("lxi h, %i", offset * 2 - 2 - _8080_cur_nlocals * 2);
    gen_emit("dad b");
}

// Params at BC + 2 + offset*2
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

void gen_param_addr(int offset)
{
    gen_emitf("lxi h, %i", offset * 2 + 2);
    gen_emit("dad b");
}

void gen_store_global(const char *name) { gen_emitf("shld %s", name); }
void gen_load_global(const char *name) { gen_emitf("lhld %s", name); }

void gen_deref(void)
{
    gen_emit("mov a, m");
    gen_emit("inx h");
    gen_emit("mov h, m");
    gen_emit("mov l, a");
}

void gen_store_to_addr(void)
{
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("mov m, a");
    gen_emit("inx h");
    gen_emit("mov a, d");
    gen_emit("mov m, a");
}

void gen_peekb(void)
{
    gen_emit("mov a, m");
    gen_emit("mov l, a");
    gen_emit("mvi h, 0");
}

void gen_pokeb(void)
{
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("mov m, a");
}

void gen_add(void) { gen_emit("dad d"); }
void gen_double(void) { gen_emit("dad h"); }

void gen_sub(void)
{
    gen_emit("xchg");
    gen_emit("mov a, e");
    gen_emit("sub l");
    gen_emit("mov l, a");
    gen_emit("mov a, d");
    gen_emit("sbb h");
    gen_emit("mov h, a");
}

void gen_mul(void)
{
    int l_loop = gen_label();
    int l_skip = gen_label();
    gen_emit("xchg");
    gen_emit("push h");
    gen_emit("pop b");
    gen_emit("lxi h, 0");
    gen_emit("mvi a, 16");
    gen_label_int(l_loop);
    gen_emit("push psw");
    gen_emit("mov a, b");
    gen_emit("rar");
    gen_emit("mov b, a");
    gen_emit("mov a, c");
    gen_emit("rar");
    gen_emit("mov c, a");
    gen_emitf("jnc .L%i", l_skip);
    gen_emit("dad d");
    gen_label_int(l_skip);
    gen_emit("xchg");
    gen_emit("dad h");
    gen_emit("xchg");
    gen_emit("pop psw");
    gen_emit("dcr a");
    gen_emitf("jnz .L%i", l_loop);
}

void gen_div(void)
{
    int l_loop = gen_label();
    int l_sub = gen_label();
    int l_done = gen_label();
    gen_emit("xchg");
    gen_emit("lxi b, 0");
    gen_emit("mvi a, 16");
    gen_label_int(l_loop);
    gen_emit("push psw");
    gen_emit("dad h");
    gen_emit("mov a, c");
    gen_emit("ral");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("ral");
    gen_emit("mov b, a");
    gen_emit("mov a, c");
    gen_emit("sub e");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("sbb d");
    gen_emit("mov b, a");
    gen_emitf("jc .L%i", l_sub);
    gen_emit("inx h");
    gen_emitf("jmp .L%i", l_done);
    gen_label_int(l_sub);
    gen_emit("mov a, c");
    gen_emit("add e");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("adc d");
    gen_emit("mov b, a");
    gen_label_int(l_done);
    gen_emit("pop psw");
    gen_emit("dcr a");
    gen_emitf("jnz .L%i", l_loop);
}

void gen_mod(void)
{
    int l_loop = gen_label();
    int l_sub = gen_label();
    int l_done = gen_label();
    gen_emit("xchg");
    gen_emit("lxi b, 0");
    gen_emit("mvi a, 16");
    gen_label_int(l_loop);
    gen_emit("push psw");
    gen_emit("dad h");
    gen_emit("mov a, c");
    gen_emit("ral");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("ral");
    gen_emit("mov b, a");
    gen_emit("mov a, c");
    gen_emit("sub e");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("sbb d");
    gen_emit("mov b, a");
    gen_emitf("jc .L%i", l_sub);
    gen_emit("inx h");
    gen_emitf("jmp .L%i", l_done);
    gen_label_int(l_sub);
    gen_emit("mov a, c");
    gen_emit("add e");
    gen_emit("mov c, a");
    gen_emit("mov a, b");
    gen_emit("adc d");
    gen_emit("mov b, a");
    gen_label_int(l_done);
    gen_emit("pop psw");
    gen_emit("dcr a");
    gen_emitf("jnz .L%i", l_loop);
    gen_emit("mov l, c");
    gen_emit("mov h, b");
}

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

void gen_not(void)
{
    gen_emit("mov a, l");
    gen_emit("cma");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("cma");
    gen_emit("mov h, a");
}

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

void gen_and(void)
{
    gen_emit("mov a, l");
    gen_emit("ana e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("ana d");
    gen_emit("mov h, a");
}

void gen_or(void)
{
    gen_emit("mov a, l");
    gen_emit("ora e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("ora d");
    gen_emit("mov h, a");
}

void gen_xor(void)
{
    gen_emit("mov a, l");
    gen_emit("xra e");
    gen_emit("mov l, a");
    gen_emit("mov a, h");
    gen_emit("xra d");
    gen_emit("mov h, a");
}

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

void gen_cmp_eq(void) { int l0=gen_label(),lm=gen_label(); gen_emit("mov a,l"); gen_emit("sub e"); gen_emitf("jnz .L%i",l0); gen_emit("mov a,h"); gen_emit("sbb d"); gen_emitf("jnz .L%i",l0); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(l0); gen_emit("lxi h,0"); gen_label_int(lm); }
void gen_cmp_ne(void) { int l0=gen_label(),lm=gen_label(); gen_emit("mov a,l"); gen_emit("sub e"); gen_emitf("jnz .L%i",l0); gen_emit("mov a,h"); gen_emit("sbb d"); gen_emit("lxi h,1"); gen_emitf("jnz .L%i",lm); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(l0); gen_emit("lxi h,1"); gen_label_int(lm); }
void gen_cmp_lt(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,1"); gen_label_int(lm); }
void gen_cmp_gt(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emitf("jz .L%i",lt); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,0"); gen_label_int(lm); }
void gen_cmp_le(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emitf("jz .L%i",lt); gen_emit("lxi h,0"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,1"); gen_label_int(lm); }
void gen_cmp_ge(void) { int lt=gen_label(),lm=gen_label(); gen_emit("xchg"); GEN_CMPCODE; gen_emitf("jc .L%i",lt); gen_emit("lxi h,1"); gen_emitf("jmp .L%i",lm); gen_label_int(lt); gen_emit("lxi h,0"); gen_label_int(lm); }

void gen_jmp(int label) { gen_emitf("jmp .L%i", label); }
void gen_jz(int label) { gen_emit("mov a, h"); gen_emit("ora l"); gen_emitf("jz .L%i", label); }
void gen_jnz(int label) { gen_emit("mov a, h"); gen_emit("ora l"); gen_emitf("jnz .L%i", label); }
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
