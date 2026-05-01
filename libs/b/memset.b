memset(ptr, val, n) {
    auto i;
    i = 0;
    while (i < n) { ptr[i] = val; i = i + 1; }
    return ptr;
}
