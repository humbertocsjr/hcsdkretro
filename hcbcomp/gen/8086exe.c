#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

/* External library routines for 32-bit math */
extern void __mul32u(void);
extern void __udiv32(void);
extern void __umod32(void);
extern void __shl32(void);
extern void __shr32(void);

int gen_label(void)
{
	return label_counter++;
}

void gen_text(void) { fprintf(out, "\nsection text\n"); }
void gen_data(void) { fprintf(out, "\nsection data\n"); }
void gen_bss(void) { fprintf(out, "\nsection bss\n"); }
void gen_global(const char *name) { fprintf(out, "global %s\n", name); }
void gen_extern(const char *name) { fprintf(out, "extern %s\n", name); }
void gen_label_str(const char *name) { fprintf(out, "%s:\n", name); }
void gen_label_int(int label) { fprintf(out, ".L%i:\n", label); }

void gen_dword(int val)
{
	fprintf(out, "\tdd %i\n", val);
}

void gen_word(int val)
{
	fprintf(out, "\tdd %i\n", val);
}

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

void gen_reserve(int n) { fprintf(out, "\tds %i\n", n * 2); }

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

static void gen_seg_override(const char *name)
{
	symbol_t *sym = lookup(name);
	if (sym && sym->segment == SEG_BSS)
	{
		gen_emit("ss");
	}
}

void gen_prologue(const char *name, int nlocals)
{
	gen_label_str(name);
	gen_emit("push bp");
	gen_emit("mov bp, sp");
	if (nlocals > 0)
	{
		gen_emitf("sub sp, %i", nlocals * 4);
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

/* Push 32-bit primary (DX:AX) */
void gen_push_prim(void)
{
	gen_emit("push dx");
	gen_emit("push ax");
}

/* Pop 32-bit secondary (CX:BX) */
void gen_pop_sec(void)
{
	gen_emit("pop bx");
	gen_emit("pop cx");
}

void gen_load_imm(int val)
{
	gen_emitf("mov ax, %i", val & 0xFFFF);
	gen_emitf("mov dx, %i", (val >> 16) & 0xFFFF);
}

/* Load 32-bit VALUE of a global variable */
void gen_load_var(const char *name)
{
	gen_seg_override(name);
	gen_emitf("mov ax, [%s]", name);
	gen_seg_override(name);
	gen_emitf("mov dx, [%s+2]", name);
}

/* Load far address of a variable/name → DX:AX = segment:offset */
void gen_load_addr(const char *name)
{
	symbol_t *sym = lookup(name);
	gen_emitf("mov ax, %s", name);
	if (sym && sym->kind == SYM_FUNCTION)
	{
		gen_emit("push cs");
		gen_emit("pop dx");
	}
	else if (sym && (sym->segment == SEG_BSS))
	{
		gen_emit("mov dx, cs");
		gen_emit("add dx, __bss_seg_delta__");
	}
	else
	{
		gen_emit("mov dx, cs");
		gen_emit("add dx, __data_seg_delta__");
	}
}

void gen_load_label(int label)
{
	gen_emitf("mov ax, .L%i", label);
	gen_emit("mov dx, cs");
	gen_emit("add dx, __data_seg_delta__");
}

void gen_store_local(int offset)
{
	gen_emitf("mov [bp-%i], ax", offset * 4 + 4);
	gen_emitf("mov [bp-%i], dx", offset * 4 + 2);
}

void gen_load_local(int offset)
{
	gen_emitf("mov ax, [bp-%i]", offset * 4 + 4);
	gen_emitf("mov dx, [bp-%i]", offset * 4 + 2);
}

void gen_local_addr(int offset)
{
	gen_emitf("lea ax, [bp-%i]", offset * 4 + 4);
	gen_emit("mov dx, ss");
}

void gen_store_param(int offset)
{
	gen_emitf("mov [bp+%i], ax", offset * 4 + 4);
	gen_emitf("mov [bp+%i], dx", offset * 4 + 6);
}

void gen_load_param(int offset)
{
	gen_emitf("mov ax, [bp+%i]", offset * 4 + 4);
	gen_emitf("mov dx, [bp+%i]", offset * 4 + 6);
}

void gen_param_addr(int offset)
{
	gen_emitf("lea ax, [bp+%i]", offset * 4 + 4);
	gen_emit("mov dx, ss");
}

/* Store 32-bit value (DX:AX) to global variable */
void gen_store_global(const char *name)
{
	gen_seg_override(name);
	gen_emitf("mov [%s], ax", name);
	gen_seg_override(name);
	gen_emitf("mov [%s+2], dx", name);
}

/* Load 32-bit VALUE of a global (same as gen_load_var for this model) */
void gen_load_global(const char *name)
{
	gen_load_var(name);
}

/* Far dereference: DX:AX = *ptr where ptr is in DX:AX */
void gen_deref(void)
{
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov ax, [es:di]");
	gen_emit("mov dx, [es:di+2]");
}

/* Store to far pointer: CX:BX = value, DX:AX = ptr */
void gen_store_to_addr(void)
{
	/* After gen_push_prim/gen_pop_sec cycle:
	 *   CX:BX = destination far pointer (segment:offset)
	 *   DX:AX = value to store (high:low)
	 * Swap so ES:DI = dest, and use AX/DX for value high/low */
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov [es:di], bx");
	gen_emit("mov [es:di+2], cx");
}

void gen_peekb(void)
{
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("xor ah, ah");
	gen_emit("mov al, [es:di]");
	gen_emit("xor dx, dx");
}

void gen_pokeb(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov [es:di], bl");
}

/* 32-bit add: DX:AX = CX:BX + DX:AX */
void gen_add(void)
{
	gen_emit("add ax, bx");
	gen_emit("adc dx, cx");
}

void gen_double(void)
{
	gen_emit("add ax, ax");
	gen_emit("adc dx, dx");
	gen_emit("add ax, ax");
	gen_emit("adc dx, dx");
}

/* 32-bit sub: DX:AX = CX:BX - DX:AX */
void gen_sub(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("sub ax, bx");
	gen_emit("sbb dx, cx");
}

/* 32-bit unsigned multiply via library */
void gen_mul(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __mul32u");
	gen_emit("add sp, 8");
}

/* 32-bit unsigned divide via library */
void gen_div(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __udiv32");
	gen_emit("add sp, 8");
}

/* 32-bit unsigned modulo via library */
void gen_mod(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __umod32");
	gen_emit("add sp, 8");
}

void gen_neg(void)
{
	gen_emit("not ax");
	gen_emit("not dx");
	gen_emit("add ax, 1");
	gen_emit("adc dx, 0");
}

void gen_not(void)
{
	gen_emit("not ax");
	gen_emit("not dx");
}

void gen_lnot(void)
{
	int l1 = gen_label();
	int l2 = gen_label();
	gen_emit("mov cx, ax");
	gen_emit("or cx, dx");
	gen_emitf("jnz .L%i", l1);
	gen_emit("mov ax, 1");
	gen_emit("mov dx, 0");
	gen_emitf("jmp .L%i", l2);
	gen_label_int(l1);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	gen_label_int(l2);
}

void gen_and(void)
{
	gen_emit("and ax, bx");
	gen_emit("and dx, cx");
}

void gen_or(void)
{
	gen_emit("or ax, bx");
	gen_emit("or dx, cx");
}

void gen_xor(void)
{
	gen_emit("xor ax, bx");
	gen_emit("xor dx, cx");
}

/* 32-bit shift left inline via library call */
void gen_shl(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __shl32");
	gen_emit("add sp, 8");
}

/* 32-bit shift right inline via library call */
void gen_shr(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __shr32");
	gen_emit("add sp, 8");
}

/* Comparison helpers */

static void gen_do_cmp(const char *jmp, int label)
{
	gen_emit("cmp dx, cx");
	gen_emitf("jne .Lcmp%i", gen_label());
	gen_emit("cmp ax, bx");
	int l = gen_label();
	gen_label_int(l);
	gen_emitf("%s .L%i", jmp, label);
	int l2 = gen_label();
	gen_emitf("jmp .L%i", l2);
	gen_label_int(l - 1);
	gen_emitf("jl .L%i", label);
	gen_label_int(l2);
}

void gen_cmp_eq(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jne .L%i", l1);
	gen_emit("cmp bx, ax");
	gen_emitf("jne .L%i", l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_emitf("jmp .L%i", lend);
	gen_label_int(l1);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	gen_label_int(lend);
}

void gen_cmp_ne(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jne .L%i", l1);
	gen_emit("cmp bx, ax");
	gen_emitf("je .L%i", lend);
	gen_label_int(l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_label_int(lend);
}

void gen_cmp_lt(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jl .L%i", l1);
	gen_emitf("jg .L%i", lend);
	gen_emit("cmp bx, ax");
	gen_emitf("jb .L%i", l1);
	gen_emitf("jmp .L%i", lend);
	gen_label_int(l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_emitf("jmp .L%i", lend + 1);
	gen_label_int(lend);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	int l2 = gen_label();
	gen_label_int(l2);
}

void gen_cmp_gt(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jg .L%i", l1);
	gen_emitf("jl .L%i", lend);
	gen_emit("cmp bx, ax");
	gen_emitf("ja .L%i", l1);
	gen_emitf("jmp .L%i", lend);
	gen_label_int(l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_emitf("jmp .L%i", lend + 1);
	gen_label_int(lend);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	int l2 = gen_label();
	gen_label_int(l2);
}

void gen_cmp_le(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jl .L%i", l1);
	gen_emitf("jg .L%i", lend);
	gen_emit("cmp bx, ax");
	gen_emitf("jbe .L%i", l1);
	gen_emitf("jmp .L%i", lend);
	gen_label_int(l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_emitf("jmp .L%i", lend + 1);
	gen_label_int(lend);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	int l2 = gen_label();
	gen_label_int(l2);
}

void gen_cmp_ge(void)
{
	int l1 = gen_label();
	int lend = gen_label();
	gen_emit("cmp cx, dx");
	gen_emitf("jg .L%i", l1);
	gen_emitf("jl .L%i", lend);
	gen_emit("cmp bx, ax");
	gen_emitf("jae .L%i", l1);
	gen_emitf("jmp .L%i", lend);
	gen_label_int(l1);
	gen_emit("mov ax, 1");
	gen_emit("xor dx, dx");
	gen_emitf("jmp .L%i", lend + 1);
	gen_label_int(lend);
	gen_emit("xor ax, ax");
	gen_emit("xor dx, dx");
	int l2 = gen_label();
	gen_label_int(l2);
}

void gen_jmp(int label)
{
	gen_emitf("jmp .L%i", label);
}

void gen_jz(int label)
{
	gen_emit("mov cx, ax");
	gen_emit("or cx, dx");
	gen_emitf("jz near .L%i", label);
}

void gen_jnz(int label)
{
	gen_emit("mov cx, ax");
	gen_emit("or cx, dx");
	gen_emitf("jnz near .L%i", label);
}

void gen_call(const char *name, int nargs)
{
	gen_emitf("call %s", name);
	if (nargs > 0)
	{
		gen_emitf("add sp, %i", nargs * 4);
	}
}

void gen_data_final(void) {}
