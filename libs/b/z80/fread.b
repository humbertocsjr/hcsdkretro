extrn ft, sb;
fread(fd, buf, nwords) {
    auto slot, i, bo, result;
    if (fd < 0 || fd >= 5) return -1;
    slot = fd * 24;
    if (ft[slot + 23] == 0) return -1;
    if (ft[slot + 21]) return 0;
    i = 0;
    while (i < nwords) {
        bo = ft[slot + 18];
        if (bo >= 128) {
            ft[slot + 19] = ft[slot + 19] + 1;
            if (ft[slot + 19] == 0) ft[slot + 20] = ft[slot + 20] + 1;
            ft[slot + 18] = 0; bo = 0;
        }
        if (bo == 0) {
            ft[slot + 16] = (ft[slot + 20] << 8) | ft[slot + 19];
            ft[slot + 17] = 0;
            set_dma(&sb);
            result = bdos(33, &ft[slot]);
            if (result != 0) { ft[slot + 21] = 1; return i; }
        }
        buf[i] = sb[ft[slot + 18] / 2];
        ft[slot + 18] = ft[slot + 18] + 2;
        i = i + 1;
    }
    return i;
}
