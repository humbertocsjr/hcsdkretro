extrn ft;
extrn name2fcb, find_slot, strc, toup, setb;
frename(oldname, newname) {
    auto fd, i, ch, result;
    fd = find_slot();
    if (fd < 0) return -1;
    name2fcb(fd, oldname);
    i = 0;
    while (i < 12) {
        ch = strc(newname, i);
        if (ch == 0) ch = ' ';
        ch = toup(ch);
        setb(&ft[fd * 24], 17 + i, ch);
        i = i + 1;
    }
    result = bdos(23, &ft[fd * 24]);
    ft[fd * 24 + 23] = 0;
    if (result == 0) return 0;
    return -1;
}
