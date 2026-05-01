extrn ft;
ftell(fd) {
    if (fd < 0 || fd >= 5) return -1;
    if (ft[fd * 24 + 23] == 0) return -1;
    return ft[fd * 24 + 19] * 128 + ft[fd * 24 + 18];
}
