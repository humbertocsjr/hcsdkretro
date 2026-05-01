extrn ft;
extrn name2fcb, find_slot;
fdelete(name) {
    auto fd, result;
    fd = find_slot();
    if (fd < 0) return -1;
    name2fcb(fd, name);
    result = bdos(19, &ft[fd * 24]);
    ft[fd * 24 + 23] = 0;
    if (result == 0) return 0;
    return -1;
}
