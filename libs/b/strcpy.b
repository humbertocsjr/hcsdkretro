strcpy(dst, src) {
    auto i;
    i = 0;
    while (src[i] != 0) { dst[i] = src[i]; i = i + 1; }
    dst[i] = 0;
    return dst;
}
