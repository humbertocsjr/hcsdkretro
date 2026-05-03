#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;
static int emit_mul = 1;
static int emit_div = 1;

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

void gen_prologue(const char *name, int nlocals)
{
    gen_label_str(name);
    gen_emit("push ix");
    gen_emit("ld ix, 0");
    gen_emit("add ix, sp");
    if (nlocals > 0)
    {
        gen_emitf("ld hl, -%i", nlocals * 2);
        gen_emit("add hl, sp");
        gen_emit("ld sp, hl");
    }
}

void gen_epilogue(void)
{
    gen_emit("ld sp, ix");
    gen_emit("pop ix");
    gen_emit("ret");
}

void gen_return(void)
{
    gen_epilogue();
}

void gen_push_prim(void)
{
    gen_emit("push hl");
}

void gen_pop_sec(void)
{
    gen_emit("pop de");
}

void gen_load_imm(int val)
{
    gen_emitf("ld hl, %i", val);
}

void gen_load_var(const char *name)
{
    gen_emitf("ld hl, [%s]", name);
}

void gen_load_addr(const char *name)
{
    gen_emitf("ld hl, %s", name);
}

void gen_load_label(int label) { fprintf(outfile, "\tld hl, .L%i\n", label); }

void gen_store_global(const char *name)
{
    gen_emitf("ld [%s], hl", name);
}

void gen_load_global(const char *name)
{
    gen_emitf("ld hl, [%s]", name);
}

void gen_store_local(int offset)
{
    gen_emitf("ld [ix+%i], l", -(offset * 2 + 2));
    gen_emitf("ld [ix+%i], h", -(offset * 2 + 1));
}

void gen_load_local(int offset)
{
    gen_emitf("ld l, [ix+%i]", -(offset * 2 + 2));
    gen_emitf("ld h, [ix+%i]", -(offset * 2 + 1));
}

void gen_local_addr(int offset)
{
    gen_emitf("ld hl, %i", -(offset * 2 + 2));
    gen_emit("ex de, hl");
    gen_emit("push ix");
    gen_emit("pop hl");
    gen_emit("add hl, de");
}
void gen_store_param(int offset)
{
    gen_emitf("ld [ix+%i], l", offset * 2 + 4);
    gen_emitf("ld [ix+%i], h", offset * 2 + 5);
}

void gen_load_param(int offset)
{
    gen_emitf("ld l, [ix+%i]", offset * 2 + 4);
    gen_emitf("ld h, [ix+%i]", offset * 2 + 5);
}

void gen_param_addr(int offset)
{
    gen_emitf("ld hl, %i", offset * 2 + 4);
    gen_emit("ex de, hl");
    gen_emit("push ix");
    gen_emit("pop hl");
    gen_emit("add hl, de");
}

void gen_deref(void)
{
    gen_emit("ld a, [hl]");
    gen_emit("inc hl");
    gen_emit("ld h, [hl]");
    gen_emit("ld l, a");
}

void gen_peekb(void)
{
    gen_emit("ld l, [hl]");
    gen_emit("ld h, 0");
}

void gen_pokeb(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld [hl], e");
}

void gen_store_to_addr(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld [hl], e");
    gen_emit("inc hl");
    gen_emit("ld [hl], d");
}

void gen_add(void)
{
    gen_emit("add hl, de");
}

void gen_double(void)
{
    gen_emit("add hl, hl");
}

void gen_sub(void)
{
    gen_emit("ex de, hl");
    gen_emit("or a");
    gen_emit("sbc hl, de");
}

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

void gen_neg(void)
{
    gen_emit("ex de, hl");
    gen_emit("ld hl, 0");
    gen_emit("or a");
    gen_emit("sbc hl, de");
}

void gen_not(void)
{
    gen_emit("ld a, l");
    gen_emit("cpl");
    gen_emit("ld l, a");
    gen_emit("ld a, h");
    gen_emit("cpl");
    gen_emit("ld h, a");
}

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

void gen_and(void)
{
    gen_emit("ld a, e");
    gen_emit("and l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("and h");
    gen_emit("ld h, a");
}

void gen_or(void)
{
    gen_emit("ld a, e");
    gen_emit("or l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("or h");
    gen_emit("ld h, a");
}

void gen_xor(void)
{
    gen_emit("ld a, e");
    gen_emit("xor l");
    gen_emit("ld l, a");
    gen_emit("ld a, d");
    gen_emit("xor h");
    gen_emit("ld h, a");
}

void gen_shl(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    // DE = value to shift, HL = shift count
    // Result: HL = DE << HL
    gen_emit("ex de, hl"); // HL = value, DE = shift count
    gen_emit("ld a, e");   // A = shift count
    gen_emit("or a");
    gen_emitf("jr z, .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("add hl, hl"); // HL <<= 1
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l_loop);
    gen_label_int(l_done);
}

void gen_shr(void)
{
    int l_done = gen_label();
    int l_loop = gen_label();
    // DE = value to shift, HL = shift count
    // Result: HL = DE >> HL
    gen_emit("ex de, hl"); // HL = value, DE = shift count
    gen_emit("ld a, e");   // A = shift count
    gen_emit("or a");
    gen_emitf("jr z, .L%i", l_done);
    gen_label_int(l_loop);
    gen_emit("srl h");
    gen_emit("rr l");
    gen_emit("dec a");
    gen_emitf("jr nz, .L%i", l_loop);
    gen_label_int(l_done);
}

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

void gen_cmp_lt(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr nc, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

void gen_cmp_gt(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr z, .L%i", l1);
    gen_emitf("jr c, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

void gen_cmp_le(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr c, .L%i", l1);
    gen_emitf("jr z, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

void gen_cmp_ge(void)
{
    int l1 = gen_label();
    gen_emit("or a");
    gen_emit("sbc hl, de");
    gen_emit("ld hl, 0");
    gen_emitf("jr c, .L%i", l1);
    gen_emit("inc hl");
    gen_label_int(l1);
}

void gen_jmp(int label)
{
    gen_emitf("jp .L%i", label);
}

void gen_jz(int label)
{
    gen_emit("ld a, h");
    gen_emit("or l");
    gen_emitf("jp z, .L%i", label);
}

void gen_jnz(int label)
{
    gen_emit("ld a, h");
    gen_emit("or l");
    gen_emitf("jp nz, .L%i", label);
}

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

