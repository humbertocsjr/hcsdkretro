extrn ft;
extrn setb, getb, strc, toup;
name2fcb(fd, name) {
    auto slot, i, ch, boff;
    slot = fd * 24;
    i = 0;
    while (i < 18) { ft[slot + i] = 0x2020; i = i + 1; }
    ft[slot] = 0;
    i = 0; boff = 1;
    while (1) {
        ch = strc(name, i);
        if (ch == 0) return;
        i = i + 1;
        if (ch == ':') {
            if (i > 1) {
                ch = strc(name, i - 2);
                ch = toup(ch);
                if (ch >= 'A' && ch <= 'P') setb(&ft[slot], 0, ch - 'A' + 1);
            }
            boff = 1;
        } else if (ch == '.') { boff = 9; }
        else {
            if (boff < 12) { ch = toup(ch); setb(&ft[slot], boff, ch); boff = boff + 1; }
        }
    }
}
