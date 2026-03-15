typedef size as u16
typedef int as s16
typedef char as u8

def main(argc as int, argv as @@char)
    var a as int
    ; comment example
    a := 1
    a += 2 if a == 1

    if a < 3
        a += 5
    else
        a += 10
    end

    while a < 10
    end

    until a < 10
    end

    a := a == 1 ? 5 : 0


end