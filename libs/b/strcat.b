strcat(dst, src) {
    auto i, j;
    i = 0;
    while (dst[i] != 0) i = i + 1;
    j = 0;
    while (src[j] != 0) { dst[i] = src[j]; i = i + 1; j = j + 1; }
    dst[i] = 0;
    return dst;
}
