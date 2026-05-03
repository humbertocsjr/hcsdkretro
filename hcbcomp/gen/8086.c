#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

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
    gen_emit("push bp");
    gen_emit("mov bp, sp");
    if (nlocals > 0)
    {
        gen_emitf("sub sp, %i", nlocals * 2);
    }
}

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
void gen_store_local(int offset)
{
    gen_emitf("mov [bp-%i], ax", offset * 2 + 2);
}

void gen_load_local(int offset)
{
    gen_emitf("mov ax, [bp-%i]", offset * 2 + 2);
}

void gen_local_addr(int offset)
{
    gen_emitf("lea ax, [bp-%i]", offset * 2 + 2);
}

void gen_store_param(int offset)
{
    gen_emitf("mov [bp+%i], ax", offset * 2 + 4);
}

void gen_load_param(int offset)
{
    gen_emitf("mov ax, [bp+%i]", offset * 2 + 4);
}

void gen_param_addr(int offset)
{
    gen_emitf("lea ax, [bp+%i]", offset * 2 + 4);
}

void gen_store_global(const char *name)
{
    gen_emitf("mov [%s], ax", name);
}

void gen_load_global(const char *name)
{
    gen_emitf("mov ax, [%s]", name);
}

void gen_deref(void)
{
    gen_emit("mov bx, ax");
    gen_emit("mov ax, [bx]");
}

void gen_store_to_addr(void)
{
    gen_emit("mov [bx], ax");
}

void gen_peekb(void)
{
    gen_emit("mov bx, ax");
    gen_emit("xor ah, ah");
    gen_emit("mov al, [bx]");
}

void gen_pokeb(void)
{
    gen_emit("mov [bx], al");
}

void gen_add(void)
{
    gen_emit("add ax, bx");
}

void gen_double(void)
{
    gen_emit("add ax, ax");
}

void gen_sub(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("sub ax, bx");
}

void gen_mul(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("mul bx");
}

void gen_div(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("cwd");
    gen_emit("div bx");
}

void gen_mod(void)
{
    gen_emit("xchg ax, bx");
    gen_emit("cwd");
    gen_emit("div bx");
    gen_emit("mov ax, dx");
}

void gen_neg(void)
{
    gen_emit("neg ax");
}

void gen_not(void)
{
    gen_emit("not ax");
}

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

void gen_and(void)
{
    gen_emit("and ax, bx");
}

void gen_or(void)
{
    gen_emit("or ax, bx");
}

void gen_xor(void)
{
    gen_emit("xor ax, bx");
}

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

void gen_call(const char *name, int nargs)
{
    gen_emitf("call %s", name);
    if (nargs > 0)
    {
        gen_emitf("add sp, %i", nargs * 2);
    }
}

void gen_reverse_args(int count)
{
    if (count <= 1) return;
    switch (count) {
    case 2:
        gen_emit("pop bx");
        gen_emit("pop ax");
        gen_emit("push bx");
        gen_emit("push ax");
        break;
    case 3:
        gen_emit("pop bx");
        gen_emit("pop cx");
        gen_emit("pop ax");
        gen_emit("push bx");
        gen_emit("push cx");
        gen_emit("push ax");
        break;
    case 4:
        gen_emit("pop bx");
        gen_emit("pop cx");
        gen_emit("pop dx");
        gen_emit("pop ax");
        gen_emit("push bx");
        gen_emit("push cx");
        gen_emit("push dx");
        gen_emit("push ax");
        break;
    case 5:
        gen_emit("pop bx");
        gen_emit("pop cx");
        gen_emit("pop dx");
        gen_emit("pop si");
        gen_emit("pop ax");
        gen_emit("push bx");
        gen_emit("push cx");
        gen_emit("push dx");
        gen_emit("push si");
        gen_emit("push ax");
        break;
    case 6:
        gen_emit("pop bx");
        gen_emit("pop cx");
        gen_emit("pop dx");
        gen_emit("pop si");
        gen_emit("pop di");
        gen_emit("pop ax");
        gen_emit("push bx");
        gen_emit("push cx");
        gen_emit("push dx");
        gen_emit("push si");
        gen_emit("push di");
        gen_emit("push ax");
        break;
    default:
        gen_emitf("; reverse %i args (unimplemented)", count);
        break;
    }
}

void gen_data_final(void) {}

