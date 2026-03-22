
def str_len(str as @char) as size
    var len as size
    len := 0
    while @str
        len += 1
        str += 1
    end
    return len
end