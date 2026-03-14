#include "../retrolang.h"


char *cpu_ext()
{
    return "8086";
}

char *cpu_name()
{
    return "8086/8088";
}

void cpu_init()
{
    // Default data types
    datatype_add("u8", NATIVETYPE_8BITS, false);
    datatype_add("s8", NATIVETYPE_8BITS, true);
    datatype_add("u16", NATIVETYPE_16BITS, false);
    datatype_add("s16", NATIVETYPE_16BITS, true);
    // Default aliases
    datatype_add("pointer", NATIVETYPE_16BITS, false);
    datatype_add("size", NATIVETYPE_16BITS, false);
    datatype_add("int", NATIVETYPE_16BITS, true);
    datatype_add("char", NATIVETYPE_8BITS, true);
}

void cpu_global_variable(char *name, int32_t size)
{
    out_line("_%s: resb %i", name, size);
}

void cpu_function_start(char *name, int32_t vars_size)
{
    out_line("_%s:", name);
    if(vars_size)
    {
        out_line("  push bp");
        out_line("  mov bp, sp");
        out_line("  sub bp, %i", (vars_size + 1) & (~1)); // round up to next word
    }
}

void cpu_function_end(char *name, int32_t vars_size)
{
    out_line("  .__END__:");
    if(vars_size)
    {
        out_line("  mov sp, bp");
        out_line("  pop bp");
    }
    out_line("  ret");
}

int32_t cpu_function_args_offset()
{
    return 4;
}

int32_t cpu_function_vars_offset()
{
    return 0;
}

void cpu_set_acc(nativetype_t nt, int32_t value)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov al, %i", value);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov ax, %i", value);
            break;
        default: error("invalid set_acc");
    }
}

void cpu_set_aux(nativetype_t nt, int32_t value)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov bl, %i", value);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov bx, %i", value);
            break;
        default: error("invalid set_acc");
    }
}

void cpu_store_global_var(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov [_%s], al", name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov [_%s], ax", name);
            break;
        default: error("invalid store_global_var");
    }
}

void cpu_store_local_var(nativetype_t nt, char *name, int32_t offset)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov [bp+%i], al ; %s", offset, name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov [bp+%i], ax ; %s", offset, name);
            break;
        default: error("invalid cpu_store_local_var");
    }
}

void cpu_load_global_var(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov al, [_%s]", name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov ax, [_%s]", name);
            break;
        default: error("invalid store_global_var");
    }
}

void cpu_load_local_var(nativetype_t nt, char *name, int32_t offset)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov al, [bp+%i] ; %s", offset, name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov ax, [bp+%i] ; %s", offset, name);
            break;
        default: error("invalid cpu_store_local_var");
    }
}

void cpu_add(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  add al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  add ax, bx");
            break;
        default: error("invalid cpu_add");
    }
}

void cpu_push_acc(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  push ax");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push ax");
            break;
        default: error("invalid cpu_push_acc");
    }
}

void cpu_pop_acc(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  pop ax");
            break;
        case NATIVETYPE_16BITS:
            out_line("  pop ax");
            break;
        default: error("invalid cpu_pop_acc");
    }
}

void cpu_push_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  push bx");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push bx");
            break;
        default: error("invalid cpu_push_aux");
    }
}

void cpu_pop_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  pop bx");
            break;
        case NATIVETYPE_16BITS:
            out_line("  pop bx");
            break;
        default: error("invalid cpu_pop_aux");
    }
}

void cpu_xchg_acc_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  xchg al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  xchg ax, bx");
            break;
        default: error("invalid cpu_xchg_acc_aux");
    }
}

void cpu_mul(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line(is_signed ? "  imul bl" : "  mul bl");
            break;
        case NATIVETYPE_16BITS:
            out_line(is_signed ? "  imul bx" : "  mul bx");
            break;
        default: error("invalid cpu_mul");
    }
}

void cpu_div(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  xor ah, ah");
            out_line(is_signed ? "  idiv bl" : "  div bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  xor dx, dx");
            out_line(is_signed ? "  idiv bx" : "  div bx");
            break;
        default: error("invalid cpu_div");
    }
}

void cpu_mod(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  xor ah, ah");
            out_line(is_signed ? "  idiv bl" : "  div bl");
            out_line("  mov al, ah");
            break;
        case NATIVETYPE_16BITS:
            out_line("  xor dx, dx");
            out_line(is_signed ? "  idiv bx" : "  div bx");
            out_line("  mov ax, dx");
            break;
        default: error("invalid cpu_mod");
    }
}

void cpu_sub(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  sub al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  sub ax, bx");
            break;
        default: error("invalid cpu_sub");
    }
}

void cpu_and(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  and al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  and ax, bx");
            break;
        default: error("invalid cpu_and");
    }
}

void cpu_or(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  or al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  or ax, bx");
            break;
        default: error("invalid cpu_or");
    }
}

void cpu_xor(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  xor al, bl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  xor ax, bx");
            break;
        default: error("invalid cpu_xor");
    }
}

char *_cpu_convert_cmp(tok_t operation, bool is_signed)
{
    switch (operation)
    {
        case TOK_EQUAL: return "e";
        case TOK_NOT_EQUAL: return "ne";
        case TOK_GREATER_THAN: return is_signed ? "g" : "a";
        case TOK_GREATER_OR_EQUAL: return is_signed ? "ge" : "ae";
        case TOK_LESS_THAN: return is_signed ? "l" : "b";
        case TOK_LESS_OR_EQUAL: return is_signed ? "le" : "be";
        default: error("invalid _cpu_conver_cmp");
    }
    return "";
}

void cpu_compare(nativetype_t nt, tok_t operation, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  cmp al, bl");
            out_line("  mov al, 0");
            out_line("  j%s $+4", _cpu_convert_cmp(operation, is_signed));
            out_line("  dec al");
            break;
        case NATIVETYPE_16BITS:
            out_line("  cmp ax, bx");
            out_line("  mov ax, 0");
            out_line("  j%s $+3", _cpu_convert_cmp(operation, is_signed));
            out_line("  dec ax");
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump_if_true(nativetype_t nt, int label)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  test al, al");
            out_line("  jne near .L%i", label);
            break;
        case NATIVETYPE_16BITS:
            out_line("  test ax, ax");
            out_line("  jne near .L%i", label);
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump_if_false(nativetype_t nt, int label)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  test al, al");
            out_line("  je near .L%i", label);
            break;
        case NATIVETYPE_16BITS:
            out_line("  test ax, ax");
            out_line("  je near .L%i", label);
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump(int label)
{
    out_line("  jmp .L%i", label);
}

void cpu_label(int label)
{
    out_line("  .L%i:", label);
}

char *cpu_comment_start()
{
    return "; ";
}

char *cpu_comment_end()
{
    return "";
}

void cpu_convet_to_8bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        case NATIVETYPE_8BITS:
            break;
        case NATIVETYPE_16BITS:
            break;
        default: error("invalid cpu_convet_to_8bit");
    }
}

void cpu_convet_to_16bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        case NATIVETYPE_8BITS:
            if(from_signed == to_signed && to_signed)
            {
                out_line("  cbw");
            }
            else
            {
                out_line("  xor ah, ah");
            }
            break;
        case NATIVETYPE_16BITS:
            break;
        default: error("invalid cpu_convet_to_16bit");
    }
}

void cpu_convet_to_24bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        default: error("invalid cpu_convet_to_24bit");
    }
}

void cpu_convet_to_32bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        default: error("invalid cpu_convet_to_32bit");
    }
}