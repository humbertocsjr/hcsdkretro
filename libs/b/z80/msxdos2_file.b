extrn bdos;

auto ft[36];
auto sb[128];

SLOT equ 6;
MAXF equ 6;

fp_l(base) return *(base + 0) & 0xFF;
fp_h(base) return (*(base + 0) >> 8) & 0xFF;
fp_get(base) return *(base + 0);

fopen(name, mode) {
    auto fd;
    fd = 0;
    while (fd < 6) {
        if (ft[fd * 3 + 2] == 0) goto found;
        fd = fd + 1;
    }
    return -1;
found:
    ft[fd * 3 + 2] = 1;
    ft[fd * 3 + 1] = mode;
    ft[fd * 3 + 0] = bdos(0x6A, name);
    if (ft[fd * 3 + 0] == 0) {
        ft[fd * 3 + 2] = 0;
        return -1;
    }
    return fd;
}

fclose(fd) {
    if (fd < 0 || fd >= 6) return -1;
    if (ft[fd * 3 + 2] == 0) return -1;
    bdos(0x6B, ft[fd * 3 + 0]);
    ft[fd * 3 + 2] = 0;
    return 0;
}

fread(fd, buf, n) {
    auto slot, result;
    if (fd < 0 || fd >= 6) return -1;
    slot = fd * 3;
    if (ft[slot + 2] == 0) return -1;
    return bdos(0x6C, buf);
}

fwrite(fd, buf, n) {
    auto slot, result;
    if (fd < 0 || fd >= 6) return -1;
    slot = fd * 3;
    if (ft[slot + 2] == 0) return -1;
    return bdos(0x6D, buf);
}
