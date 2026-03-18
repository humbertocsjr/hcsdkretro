var a as @@u8, b as @u16[128]
var c as @u8, d as u8[123-100-20]

decl var_externa as int
decl multiplicacao(a as int, b as int) as int

decl tty_print_char(c as char)
decl tty_print_str(str as @char)

def main()
    var a as s8, b as s8
    var ptr as pointer
    var indi_a as @s8
    a := a < 3 || 1 if 1
    if 1
        a := 1
    else
        a := 2
    end
    a := 0
    until a == 3
        a += 1
    end
    a := 0
    while a < 3
        a += 1
    end
    asm z80 "; Z80"
    asm 8086 "; 8086"
    ptr := a + 1 + b + soma ; test
    ptr := soma
    a := soma(1,2) + ptr()
    indi_a := addressof(a)
    b := @indi_a
    tty_print_str("Oie")
    @indi_a := b
end

def soma(a as int, b as int)
    return a + b
end