extrn ft, sb;
fwrite(fd, buf, nwords) {
    auto slot, i, bo, result;
    if (fd < 0 || fd >= 5) return -1;
    slot = fd * 24;
    if (ft[slot + 23] == 0) return -1;
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
        }
        sb[ft[slot + 18] / 2] = buf[i];
        ft[slot + 18] = ft[slot + 18] + 2;
        i = i + 1;
        if (ft[slot + 18] >= 128 || i == nwords) {
            set_dma(&sb);
            result = bdos(34, &ft[slot]);
        }
    }
    return i;
}
