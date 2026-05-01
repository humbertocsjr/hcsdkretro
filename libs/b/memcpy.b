memcpy(dst, src, n) {
    auto i;
    i = 0;
    while (i < n) { dst[i] = src[i]; i = i + 1; }
    return dst;
}
