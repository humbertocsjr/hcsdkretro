var a as @@u8, b as @u16[128]
var c as @u8, d as u8[123-100-20]

def main()
    var a as s8
    var ptr as pointer
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
    
end

def soma()

end