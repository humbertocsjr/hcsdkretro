extrn ft;
extrn name2fcb, find_slot;
extrn setb, getb, strc, toup;
fopen(name, mode) {
    auto fd, slot, result;
    fd = find_slot();
    if (fd < 0) return -1;
    slot = fd * 24;
    name2fcb(fd, name);
    ft[slot + 23] = 1;
    ft[slot + 22] = mode;
    ft[slot + 18] = 0; ft[slot + 19] = 0;
    ft[slot + 20] = 0; ft[slot + 21] = 0;
    if (mode == 0 || mode == 2) {
        result = bdos(15, &ft[slot]);
        if (result != 0) { ft[slot + 23] = 0; return -1; }
    }
    if (mode == 1) {
        result = bdos(22, &ft[slot]);
        if (result != 0) { ft[slot + 23] = 0; return -1; }
    }
    if (mode == 2) {
        result = bdos(35, &ft[slot]);
        ft[slot + 19] = ft[slot + 16];
        ft[slot + 20] = ft[slot + 17];
        ft[slot + 18] = 0;
    }
    return fd;
}
