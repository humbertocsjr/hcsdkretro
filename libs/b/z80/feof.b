extrn ft;
feof(fd) {
    if (fd < 0 || fd >= 5) return -1;
    return ft[fd * 24 + 21];
}
