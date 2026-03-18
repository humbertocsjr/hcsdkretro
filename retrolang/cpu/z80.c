#include "../retrolang.h"


char *cpu_ext()
{
    return "z80";
}

char *cpu_name()
{
    return "Z80";
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
    out_line("section data");
    out_line("global _%s", name);
    out_line("_%s: resb %i", name, size);
}

void cpu_function_start(char *name, int32_t vars_size)
{
    out_line("section text");
    out_line("global _%s", name);
    out_line("_%s:", name);
    if(vars_size)
    {
        out_line("  push ix");
        out_line("  ld ix, -%i", (vars_size + 1) & (~1)); // round up to next word
        out_line("  add ix, sp");
    }
}

void cpu_function_end(char *name, int32_t vars_size)
{
    out_line("  .__END__:");
    if(vars_size)
    {
        out_line("  ld sp, ix");
        out_line("  pop ix");
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
            out_line("  ld a, %i", value);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld hl, %i", value);
            break;
        default: error("invalid set_acc");
    }
}

void cpu_set_aux(nativetype_t nt, int32_t value)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld e, %i", value);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld de, %i", value);
            break;
        default: error("invalid set_acc");
    }
}

void cpu_store_global_var(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld [_%s], a", name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld [_%s], hl", name);
            break;
        default: error("invalid store_global_var");
    }
}

void cpu_store_local_var(nativetype_t nt, char *name, int32_t offset)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld [ix+%i], a ; %s", offset, name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld [ix+%i], l ; %s", offset, name);
            out_line("  ld [ix+%i], h ; %s", offset+1, name);
            break;
        default: error("invalid cpu_store_local_var");
    }
}

void cpu_load_global_var(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld a, [_%s]", name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld hl, [_%s]", name);
            break;
        default: error("invalid store_global_var");
    }
}

void cpu_load_local_var(nativetype_t nt, char *name, int32_t offset)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld a, [ix+%i] ; %s", offset, name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld l, [ix+%i] ; %s", offset, name);
            out_line("  ld h, [ix+%i] ; %s", offset+1, name);
            break;
        default: error("invalid cpu_store_local_var");
    }
}

void cpu_load_aux_global_var(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld e, [_%s]", name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld de, [_%s]", name);
            break;
        default: error("invalid store_global_var");
    }
}

void cpu_load_aux_local_var(nativetype_t nt, char *name, int32_t offset)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  mov e, [ix+%i] ; %s", offset, name);
            break;
        case NATIVETYPE_16BITS:
            out_line("  mov e, [ix+%i] ; %s", offset, name);
            out_line("  mov d, [ix+%i] ; %s", offset+1, name);
            break;
        default: error("invalid cpu_store_local_var");
    }
}

void cpu_add(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  add a, e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  add hl, de");
            break;
        default: error("invalid cpu_add");
    }
}

void cpu_push_acc(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld l, a");
            out_line("  push hl");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push hl");
            break;
        default: error("invalid cpu_push_acc");
    }
}

void cpu_pop_acc(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  pop hl");
            out_line("  ld a, l");
            break;
        case NATIVETYPE_16BITS:
            out_line("  pop hl");
            break;
        default: error("invalid cpu_pop_acc");
    }
}

void cpu_push_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  push de");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push de");
            break;
        default: error("invalid cpu_push_aux");
    }
}

void cpu_pop_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  pop de");
            break;
        case NATIVETYPE_16BITS:
            out_line("  pop de");
            break;
        default: error("invalid cpu_pop_aux");
    }
}

void cpu_xchg_acc_aux(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld l, a");
            out_line("  ex hl, de");
            out_line("  ld a, l");
            break;
        case NATIVETYPE_16BITS:
            out_line("  ex hl, de");
            break;
        default: error("invalid cpu_xchg_acc_aux");
    }
}

void cpu_mul(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line(is_signed ? "  call muls8" : "  call mulu8");
            break;
        case NATIVETYPE_16BITS:
            out_line(is_signed ? "  call muls16" : "  call mulu16");
            break;
        default: error("invalid cpu_mul");
    }
}

void cpu_div(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line(is_signed ? "  call divs8" : "  call divu8");
            break;
        case NATIVETYPE_16BITS:
            out_line(is_signed ? "  call divs16" : "  call divu16");
            break;
        default: error("invalid cpu_div");
    }
}

void cpu_mod(nativetype_t nt, bool is_signed)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line(is_signed ? "  call mods8" : "  call modu8");
            break;
        case NATIVETYPE_16BITS:
            out_line(is_signed ? "  call mods16" : "  call modu16");
            break;
        default: error("invalid cpu_mod");
    }
}

void cpu_sub(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  sub a, e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  or a");
            out_line("  sbc hl, de");
            break;
        default: error("invalid cpu_sub");
    }
}

void cpu_and(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  and e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld a, l");
            out_line("  and e");
            out_line("  ld l, a");
            out_line("  ld a, h");
            out_line("  and d");
            out_line("  ld h, a");
            break;
        default: error("invalid cpu_and");
    }
}

void cpu_or(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  or e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld a, l");
            out_line("  or e");
            out_line("  ld l, a");
            out_line("  ld a, h");
            out_line("  or d");
            out_line("  ld h, a");
            break;
        default: error("invalid cpu_or");
    }
}

void cpu_xor(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  xor e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  ld a, l");
            out_line("  xor e");
            out_line("  ld l, a");
            out_line("  ld a, h");
            out_line("  xor d");
            out_line("  ld h, a");
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
            out_line("  call cmp%s8", _cpu_convert_cmp(operation, is_signed));
            break;
        case NATIVETYPE_16BITS:
            out_line("  call cmp%s16", _cpu_convert_cmp(operation, is_signed));
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump_if_true(nativetype_t nt, int label)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  call cmpe8");
            out_line("  jp nz, .L%i", label);
            break;
        case NATIVETYPE_16BITS:
            out_line("  call cmpe16");
            out_line("  jp nz, .L%i", label);
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump_if_false(nativetype_t nt, int label)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  call cmpe8");
            out_line("  jp z, .L%i", label);
            break;
        case NATIVETYPE_16BITS:
            out_line("  call cmpe16");
            out_line("  jp z, .L%i", label);
            break;
        default: error("invalid cpu_comparsion");
    }
}

void cpu_jump(int label)
{
    out_line("  jp .L%i", label);
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

void cpu_convert_to_8bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        case NATIVETYPE_8BITS:
            break;
        case NATIVETYPE_16BITS:
            break;
        default: error("invalid cpu_convert_to_8bit");
    }
}

void cpu_convert_to_16bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        case NATIVETYPE_8BITS:
            if(from_signed == to_signed && to_signed)
            {
                out_line("  call cbw");
            }
            else
            {
                out_line("  ld l, a");
                out_line("  ld a, 0");
                out_line("  ld h, a");
            }
            break;
        case NATIVETYPE_16BITS:
            break;
        default: error("invalid cpu_convert_to_16bit");
    }
}

void cpu_convert_to_24bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        default: error("invalid cpu_convert_to_24bit");
    }
}

void cpu_convert_to_32bit(nativetype_t from, bool from_signed, bool to_signed)
{
    switch(from)
    {
        default: error("invalid cpu_convert_to_32bit");
    }
}

void cpu_set_acc_as_pointer(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("  ld hl, _%s", name);
            break;
        default: error("invalid cpu_set_acc_as_pointer");
    }
}

void cpu_set_aux_as_pointer(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("  ld de, _%s", name);
            break;
        default: error("invalid cpu_set_acc_as_pointer");
    }
}

void cpu_call_function(nativetype_t nt, char *name)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
        case NATIVETYPE_16BITS:
            out_line("  call _%s", name);
            break;
        default: error("invalid cpu_call_function");
    }
}

void cpu_call_acc()
{
    out_line("  call ax");
}

void cpu_restore_stack(int32_t size)
{
    if(size > 0) out_line("  ld iy, %i", size);
    if(size < 0) out_line("  ld iy, %i", -size);
    if(size) out_line("  add iy, sp");
    if(size) out_line("  ld sp, iy");
}

int cpu_get_stack_align()
{
    return 2;
}

void cpu_return_from_function()
{
    out_line("  jp .__END__");
}

void cpu_load_pointer(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld a, [hl]");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push hl");
            out_line("  pop iy");
            out_line("  ld l, [iy+0]");
            out_line("  ld h, [iy+1]");
            break;
        default: error("invalid cpu_load_pointer");
    }
}

void cpu_set_acc_pointer_local(nativetype_t nt, char *name, int offset)
{
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("  ld hl, %i ; %s", offset, name);
            out_line("  add hl, sp");
            break;
        default: error("invalid cpu_load_pointer");
    }
}

void cpu_set_aux_pointer_local(nativetype_t nt, char *name, int offset)
{
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("  ex de, hl");
            out_line("  ld hl, %i ; %s", offset, name);
            out_line("  add hl, sp");
            out_line("  ex de, hl");
            break;
        default: error("invalid cpu_load_pointer");
    }
}

void cpu_set_acc_as_string(nativetype_t nt, char *string)
{
    int lbl = new_label();
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("section data");
            out_line("  .L%i:", lbl);
            while(*string)
            {
                out_line("    db %i ; %c", *string, *string < 32 && *string > 127 ? '?' : *string);
                string++;
            }
            out_line("    db 0");
            out_line("section text");
            out_line("  ld hl, .L%i", lbl);
            break;
        default: error("invalid cpu_set_acc_as_string");
    }
}

void cpu_set_aux_as_string(nativetype_t nt, char *string)
{
    int lbl = new_label();
    switch(nt)
    {
        case NATIVETYPE_16BITS:
            out_line("section data");
            out_line("  .L%i:", lbl);
            while(*string)
            {
                out_line("    db %i ; %c", *string, *string < 32 && *string > 127 ? '?' : *string);
                string++;
            }
            out_line("    db 0");
            out_line("section text");
            out_line("  ld de, .L%i", lbl);
            break;
        default: error("invalid cpu_set_aux_as_string");
    }
}

void cpu_store_aux_to_acc_pointer(nativetype_t nt)
{
    switch(nt)
    {
        case NATIVETYPE_8BITS:
            out_line("  ld [hl], e");
            break;
        case NATIVETYPE_16BITS:
            out_line("  push hl");
            out_line("  pop iy");
            out_line("  ld [iy+0], e");
            out_line("  ld [iy+1], d");
            break;
        default: error("invalid cpu_store_aux_to_acc_pointer");
    }
}