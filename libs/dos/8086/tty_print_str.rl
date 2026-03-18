
decl tty_print_char(c as char)

def tty_print_str(str as @char)
    var c as char
    while @str
        tty_print_char(@str)
        str += 1
    end
end