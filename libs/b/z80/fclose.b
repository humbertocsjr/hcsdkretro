extrn ft;
fclose(fd) {
    if (fd < 0 || fd >= 5) return -1;
    if (ft[fd * 24 + 23] == 0) return -1;
    bdos(16, &ft[fd * 24]);
    ft[fd * 24 + 23] = 0;
    return 0;
}
