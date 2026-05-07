#include "../bcomp.h"

extern FILE *outfile;
#define out outfile
static int label_counter = 0;

/* External library routines for 32-bit math / Rotinas de biblioteca externas para matemática de 32 bits */
extern void __mul32u(void);
extern void __udiv32(void);
extern void __umod32(void);
extern void __shl32(void);
extern void __shr32(void);

// [English] Generates a new unique label number for 8086exe (32-bit far) assembly
// [Portuguese] Gera um novo número de rótulo único para assembly 8086exe (32-bit far)
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

// [English] Emits a 32-bit doubleword value (DD directive)
// [Portuguese] Emite um valor doubleword de 32 bits (diretiva DD)
void gen_dword(int val)
{
	fprintf(out, "\tdd %i\n", val);
}

// [English] In 8086exe mode, all words are 32-bit: emits DD (not DW)
// [Portuguese] No modo 8086exe, todas as words são 32-bit: emite DD (não DW)
void gen_word(int val)
{
	fprintf(out, "\tdd %i\n", val);
}

// [English] Emits a string as a DB directive with escape sequence handling for 8086exe
// [Portuguese] Emite uma string como diretiva DB com tratamento de sequências de escape para 8086exe
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

// [English] Reserves space: in 8086exe mode each unit is 4 bytes (32-bit)
// [Portuguese] Reserva espaço: no modo 8086exe cada unidade tem 4 bytes (32-bit)
void gen_reserve(int n) { fprintf(out, "\tds %i\n", n * 2); }

// [English] Emits a formatted comment line in 8086exe assembly
// [Portuguese] Emite uma linha de comentário formatada em assembly 8086exe
void gen_comment(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(out, "\t; ");
	vfprintf(out, fmt, args);
	fprintf(out, "\n");
	va_end(args);
}

// [English] Emits a raw 8086exe assembly instruction line
// [Portuguese] Emite uma linha de instrução assembly 8086exe bruta
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

// [English] Generates segment override prefix for BSS variables
// [Portuguese] Gera prefixo de override de segmento para variáveis BSS
static void gen_seg_override(const char *name)
{
	symbol_t *sym = lookup(name);
	if (sym && sym->segment == SEG_BSS)
	{
		gen_emit("ss");
	}
}

// [English] Generates function prologue for 8086exe: push bp, mov bp/sp, allocate 32-bit locals
// [Portuguese] Gera prólogo de função para 8086exe: push bp, mov bp/sp, aloca locais de 32 bits
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

// [English] Generates function epilogue for 8086exe: mov sp/bp, pop bp, ret
// [Portuguese] Gera epílogo de função para 8086exe: mov sp/bp, pop bp, ret
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

// [English] Pushes 32-bit primary (DX:AX) onto the stack
// [Portuguese] Empilha o primário de 32 bits (DX:AX) na pilha
void gen_push_prim(void)
{
	gen_emit("push dx");
	gen_emit("push ax");
}

// [English] Pops 32-bit secondary (CX:BX) from the stack
// [Portuguese] Desempilha o secundário de 32 bits (CX:BX) da pilha
void gen_pop_sec(void)
{
	gen_emit("pop bx");
	gen_emit("pop cx");
}

// [English] Exchanges 32-bit primary (DX:AX) and secondary (CX:BX) registers
// [Portuguese] Troca os registradores primário (DX:AX) e secundário (CX:BX) de 32 bits
void gen_exchange(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
}

// [English] Loads a 32-bit immediate value into DX:AX
// [Portuguese] Carrega um valor imediato de 32 bits em DX:AX
void gen_load_imm(int val)
{
	gen_emitf("mov ax, %i", val & 0xFFFF);
	gen_emitf("mov dx, %i", (val >> 16) & 0xFFFF);
}

// [English] Loads 32-bit VALUE of a global variable into DX:AX
// [Portuguese] Carrega o VALOR de 32 bits de uma variável global em DX:AX
void gen_load_var(const char *name)
{
	gen_seg_override(name);
	gen_emitf("mov ax, [%s]", name);
	gen_seg_override(name);
	gen_emitf("mov dx, [%s+2]", name);
}

// [English] Loads far address of a variable/name into DX:AX = segment:offset.
// Handles functions (CS), BSS (CS + bss_delta), and DATA (CS + data_delta).
// [Portuguese] Carrega endereço far de uma variável/nome em DX:AX = segment:offset.
// Trata funções (CS), BSS (CS + bss_delta) e DATA (CS + data_delta).
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

// [English] Loads the far address of a compiler-generated label into DX:AX
// [Portuguese] Carrega o endereço far de um rótulo gerado pelo compilador em DX:AX
void gen_load_label(int label)
{
	gen_emitf("mov ax, .L%i", label);
	gen_emit("mov dx, cs");
	gen_emit("add dx, __data_seg_delta__");
}

// [English] Stores 32-bit DX:AX to a local variable at BP-offset
// [Portuguese] Armazena DX:AX de 32 bits em uma variável local no deslocamento BP-offset
void gen_store_local(int offset)
{
	gen_emitf("mov [bp-%i], ax", offset * 4 + 4);
	gen_emitf("mov [bp-%i], dx", offset * 4 + 2);
}

// [English] Stores an immediate 32-bit value to a local variable at BP-offset
// Optimized: generates "mov ax,LO; mov dx,HI; mov [bp-OFF],ax; mov [bp-OFF+2],dx"
// [Portuguese] Armazena valor imediato de 32 bits em variável local no BP-offset
void gen_store_imm_local(int val, int offset)
{
    int off_lo = offset * 4 + 4;
    int off_hi = offset * 4 + 2;
    int lo = val & 0xFFFF;
    int hi = (val >> 16) & 0xFFFF;
    
    if (val == 0) {
        gen_emit("xor ax, ax");
        gen_emit("xor dx, dx");
    } else {
        gen_emitf("mov ax, %i", lo);
        gen_emitf("mov dx, %i", hi);
    }
    gen_emitf("mov [bp-%i], ax", off_lo);
    gen_emitf("mov [bp-%i], dx", off_hi);
}

// [English] Loads 32-bit DX:AX from a local variable at BP-offset
// [Portuguese] Carrega DX:AX de 32 bits de uma variável local no deslocamento BP-offset
void gen_load_local(int offset)
{
	gen_emitf("mov ax, [bp-%i]", offset * 4 + 4);
	gen_emitf("mov dx, [bp-%i]", offset * 4 + 2);
}

// [English] Computes far address of a local variable: LEA AX BP-offset, DX = SS
// [Portuguese] Computa endereço far de variável local: LEA AX BP-offset, DX = SS
void gen_local_addr(int offset)
{
	gen_emitf("lea ax, [bp-%i]", offset * 4 + 4);
	gen_emit("mov dx, ss");
}

// [English] Stores 32-bit DX:AX to a parameter at BP+offset
// [Portuguese] Armazena DX:AX de 32 bits em um parâmetro no deslocamento BP+offset
void gen_store_param(int offset)
{
	gen_emitf("mov [bp+%i], ax", offset * 4 + 4);
	gen_emitf("mov [bp+%i], dx", offset * 4 + 6);
}

// [English] Loads 32-bit DX:AX from a parameter at BP+offset
// [Portuguese] Carrega DX:AX de 32 bits de um parâmetro no deslocamento BP+offset
void gen_load_param(int offset)
{
	gen_emitf("mov ax, [bp+%i]", offset * 4 + 4);
	gen_emitf("mov dx, [bp+%i]", offset * 4 + 6);
}

// [English] Computes far address of a parameter: LEA AX BP+offset, DX = SS
// [Portuguese] Computa endereço far de parâmetro: LEA AX BP+offset, DX = SS
void gen_param_addr(int offset)
{
	gen_emitf("lea ax, [bp+%i]", offset * 4 + 4);
	gen_emit("mov dx, ss");
}

// [English] Stores 32-bit value (DX:AX) to a global variable with segment override
// [Portuguese] Armazena valor de 32 bits (DX:AX) em variável global com override de segmento
void gen_store_global(const char *name)
{
	gen_seg_override(name);
	gen_emitf("mov [%s], ax", name);
	gen_seg_override(name);
	gen_emitf("mov [%s+2], dx", name);
}

// [English] Loads 32-bit value from a global (same as gen_load_var)
// [Portuguese] Carrega valor de 32 bits de uma global (mesmo que gen_load_var)
void gen_load_global(const char *name)
{
	gen_load_var(name);
}

// [English] Far dereference: loads 32-bit value from far pointer DX:AX into DX:AX
// [Portuguese] Dereferência far: carrega valor de 32 bits do ponteiro far DX:AX em DX:AX
void gen_deref(void)
{
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov ax, [es:di]");
	gen_emit("mov dx, [es:di+2]");
}

// [English] Stores 32-bit value to far pointer. CX:BX = dest ptr, DX:AX = value.
// Swaps so ES:DI = dest, then stores value low/high.
// [Portuguese] Armazena valor de 32 bits em ponteiro far. CX:BX = ptr dest, DX:AX = valor.
// Troca para ES:DI = dest, então armazena valor baixo/alto.
void gen_store_to_addr(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov [es:di], bx");
	gen_emit("mov [es:di+2], cx");
}

// [English] Reads a byte from far pointer DX:AX (peekb), zero-extends to 32 bits
// [Portuguese] Lê um byte de ponteiro far DX:AX (peekb), estende com zero para 32 bits
void gen_peekb(void)
{
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("xor ah, ah");
	gen_emit("mov al, [es:di]");
	gen_emit("xor dx, dx");
}

// [English] Writes a byte (pokeb) to far pointer DX:AX via CX:BX swap
// [Portuguese] Escreve um byte (pokeb) em ponteiro far DX:AX via troca CX:BX
void gen_pokeb(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("mov di, ax");
	gen_emit("mov es, dx");
	gen_emit("mov [es:di], bl");
}

// [English] 32-bit addition: DX:AX = CX:BX + DX:AX
// [Portuguese] Adição de 32 bits: DX:AX = CX:BX + DX:AX
void gen_add(void)
{
	gen_emit("add ax, bx");
	gen_emit("adc dx, cx");
}

// [English] 32-bit double (multiply by 2): DX:AX = DX:AX * 2
// Note: this does add hl,hl twice which is DX:AX *= 4, not 2.
// [Portuguese] Duplicação de 32 bits (multiplicar por 2): DX:AX = DX:AX * 2
void gen_double(void)
{
	gen_emit("add ax, ax");
	gen_emit("adc dx, dx");
	gen_emit("add ax, ax");
	gen_emit("adc dx, dx");
}

// [English] 32-bit subtraction: DX:AX = CX:BX - DX:AX
// [Portuguese] Subtração de 32 bits: DX:AX = CX:BX - DX:AX
void gen_sub(void)
{
	gen_emit("xchg ax, bx");
	gen_emit("xchg dx, cx");
	gen_emit("sub ax, bx");
	gen_emit("sbb dx, cx");
}

// [English] 32-bit unsigned multiplication via library call
// [Portuguese] Multiplicação unsigned de 32 bits via chamada de biblioteca
void gen_mul(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __mul32u");
	gen_emit("add sp, 8");
}

// [English] 32-bit unsigned division via library call
// [Portuguese] Divisão unsigned de 32 bits via chamada de biblioteca
void gen_div(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __udiv32");
	gen_emit("add sp, 8");
}

// [English] 32-bit unsigned modulo via library call
// [Portuguese] Módulo unsigned de 32 bits via chamada de biblioteca
void gen_mod(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __umod32");
	gen_emit("add sp, 8");
}

// [English] 32-bit negation (two's complement): DX:AX = -DX:AX
// [Portuguese] Negação de 32 bits (complemento de dois): DX:AX = -DX:AX
void gen_neg(void)
{
	gen_emit("not ax");
	gen_emit("not dx");
	gen_emit("add ax, 1");
	gen_emit("adc dx, 0");
}

// [English] 32-bit bitwise NOT: DX:AX = ~DX:AX
// [Portuguese] NOT bitwise de 32 bits: DX:AX = ~DX:AX
void gen_not(void)
{
	gen_emit("not ax");
	gen_emit("not dx");
}

// [English] 32-bit logical NOT: DX:AX = !DX:AX (returns 0 or 1)
// [Portuguese] NOT lógico de 32 bits: DX:AX = !DX:AX (retorna 0 ou 1)
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

// [English] 32-bit bitwise AND: DX:AX = DX:AX & CX:BX
// [Portuguese] AND bitwise de 32 bits: DX:AX = DX:AX & CX:BX
void gen_and(void)
{
	gen_emit("and ax, bx");
	gen_emit("and dx, cx");
}

// [English] 32-bit bitwise OR: DX:AX = DX:AX | CX:BX
// [Portuguese] OR bitwise de 32 bits: DX:AX = DX:AX | CX:BX
void gen_or(void)
{
	gen_emit("or ax, bx");
	gen_emit("or dx, cx");
}

// [English] 32-bit bitwise XOR: DX:AX = DX:AX ^ CX:BX
// [Portuguese] XOR bitwise de 32 bits: DX:AX = DX:AX ^ CX:BX
void gen_xor(void)
{
	gen_emit("xor ax, bx");
	gen_emit("xor dx, cx");
}

// [English] 32-bit left shift via library call
// [Portuguese] Deslocamento à esquerda de 32 bits via chamada de biblioteca
void gen_shl(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __shl32");
	gen_emit("add sp, 8");
}

// [English] 32-bit right shift via library call
// [Portuguese] Deslocamento à direita de 32 bits via chamada de biblioteca
void gen_shr(void)
{
	gen_emit("push cx");
	gen_emit("push bx");
	gen_emit("push dx");
	gen_emit("push ax");
	gen_emit("call __shr32");
	gen_emit("add sp, 8");
}

// [English] Internal helper: generates 32-bit comparison with high word check and jump
// [Portuguese] Auxiliar interno: gera comparação de 32 bits com verificação de high word e salto
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

// [English] 32-bit equality comparison: DX:AX = (CX:BX == DX:AX) ? 1 : 0
// [Portuguese] Comparação de igualdade de 32 bits: DX:AX = (CX:BX == DX:AX) ? 1 : 0
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

// [English] 32-bit not-equal comparison: DX:AX = (CX:BX != DX:AX) ? 1 : 0
// [Portuguese] Comparação de desigualdade de 32 bits: DX:AX = (CX:BX != DX:AX) ? 1 : 0
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

// [English] 32-bit less-than comparison (unsigned): DX:AX = (CX:BX < DX:AX) ? 1 : 0
// [Portuguese] Comparação menor-que de 32 bits (unsigned): DX:AX = (CX:BX < DX:AX) ? 1 : 0
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

// [English] 32-bit greater-than comparison (unsigned): DX:AX = (CX:BX > DX:AX) ? 1 : 0
// [Portuguese] Comparação maior-que de 32 bits (unsigned): DX:AX = (CX:BX > DX:AX) ? 1 : 0
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

// [English] 32-bit less-or-equal comparison (unsigned): DX:AX = (CX:BX <= DX:AX) ? 1 : 0
// [Portuguese] Comparação menor-ou-igual de 32 bits (unsigned): DX:AX = (CX:BX <= DX:AX) ? 1 : 0
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

// [English] 32-bit greater-or-equal comparison (unsigned): DX:AX = (CX:BX >= DX:AX) ? 1 : 0
// [Portuguese] Comparação maior-ou-igual de 32 bits (unsigned): DX:AX = (CX:BX >= DX:AX) ? 1 : 0
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

// [English] Generates function call and adjusts stack for 32-bit arguments on 8086exe
// [Portuguese] Gera chamada de função e ajusta a pilha para argumentos de 32 bits no 8086exe
void gen_call(const char *name, int nargs)
{
	gen_emitf("call %s", name);
	if (nargs > 0)
	{
		gen_emitf("add sp, %i", nargs * 4);
	}
}

void gen_data_final(void) {}

// [English] 8086exe Pattern 1: replaces "push dx; push ax; pop bx; pop cx" with "mov cx, dx; mov bx, ax"
// [Portuguese] Padrão 8086exe 1: substitui "push dx; push ax; pop bx; pop cx" por "mov cx, dx; mov bx, ax"
static int i86e_match_push32_pop32(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "push", 1)) return 0;
    if (!peep_op_args(&w[1], "push", 1)) return 0;
    if (!peep_op_args(&w[2], "pop", 1)) return 0;
    if (!peep_op_args(&w[3], "pop", 1)) return 0;
    if (!strcmp(w[0].args[0], w[3].args[0]) && !strcmp(w[1].args[0], w[2].args[0]))
        return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tmov %s, %s", w[3].args[0], w[0].args[0]);
    peep_emit_repl(repl, &n, "\tmov %s, %s", w[2].args[0], w[1].args[0]);
    return n;
}

// [English] 8086exe Pattern 2: collapses "lea ax,[bp+N]; mov bx,ax; mov ax,[bx]" into "mov ax,[bp+N]"
// [Portuguese] Padrão 8086exe 2: colapsa ... em "mov ax,[bp+N]"
static int i86e_match_lea_deref(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "lea", 2)) return 0;
    if (strcmp(w[0].args[0], "ax")) return 0;
    const char *bp_expr = w[0].args[1];
    if (strncmp(bp_expr, "[bp", 3)) return 0;
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
    peep_emit_repl(repl, &n, "\tmov ax, %s", bp_expr);
    return n;
}

// [English] 8086exe Pattern 3: replaces "mov dx, 0" with "xor dx, dx" (shorter, same effect).
// [Portuguese] Padrão 8086exe 3: substitui "mov dx, 0" por "xor dx, dx" (mais curto, mesmo efeito).
static int i86e_match_mov_dx_zero(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "mov", 2)) return 0;
    if (strcmp(w[0].args[0], "dx") || strcmp(w[0].args[1], "0")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\txor dx, dx");
    return n;
}

// [English] 8086exe Pattern 4: replaces "not ax; not dx; add ax,1; adc dx,0" with
// "neg dx; neg ax; sbb dx,0" (smaller, same effect for 32-bit negation).
// [Portuguese] Padrão 8086exe 4: substitui sequência de negação 32-bit por versão menor.
static int i86e_match_neg32(peep_line_t *w, peep_line_t *repl)
{
    if (!peep_op_args(&w[0], "not", 1) || strcmp(w[0].args[0], "ax")) return 0;
    if (!peep_op_args(&w[1], "not", 1) || strcmp(w[1].args[0], "dx")) return 0;
    if (!peep_op_args(&w[2], "add", 2) || strcmp(w[2].args[0], "ax") || strcmp(w[2].args[1], "1")) return 0;
    if (!peep_op_args(&w[3], "adc", 2) || strcmp(w[3].args[0], "dx") || strcmp(w[3].args[1], "0")) return 0;
    int n = 0;
    peep_emit_repl(repl, &n, "\tneg dx");
    peep_emit_repl(repl, &n, "\tneg ax");
    peep_emit_repl(repl, &n, "\tsbb dx, 0");
    return n;
}

// [English] 8086exe-specific peephole pattern dispatcher.
// [Portuguese] Despachante de padrões peephole específicos 8086exe.
int gen_peep_replace(peep_line_t *window, int wcount, peep_line_t *repl)
{
    int n;
    if (wcount == 1) {
        n = i86e_match_mov_dx_zero(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 3) {
        n = i86e_match_lea_deref(window, repl);
        if (n > 0) return n;
    }
    if (wcount == 4) {
        n = i86e_match_push32_pop32(window, repl);
        if (n > 0) return n;
        n = i86e_match_neg32(window, repl);
        if (n > 0) return n;
    }
    return 0;
}
