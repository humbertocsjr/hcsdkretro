extrn ft;
fseek(fd, offset, whence) {
    auto slot, result;
    if (fd < 0 || fd >= 5) return -1;
    slot = fd * 24;
    if (ft[slot + 23] == 0) return -1;
    if (whence == 0) {
        ft[slot + 19] = offset / 128;
        ft[slot + 18] = offset % 128;
        ft[slot + 20] = 0;
        ft[slot + 21] = 0;
        return 0;
    }
    if (whence == 2) {
        result = bdos(35, &ft[slot]);
        ft[slot + 19] = ft[slot + 16];
        ft[slot + 20] = ft[slot + 17];
        ft[slot + 18] = 0;
        ft[slot + 21] = 0;
        return 0;
    }
    return -1;
}
