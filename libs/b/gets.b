extrn getchar;
gets(s) {
    auto i, c;
    i = 0;
    while (i < 254) {
        c = getchar();
        if (c == '\n') break;
        if (c == '\r') break;
        if (c == -1) break;
        s[i] = c;
        i = i + 1;
    }
    s[i] = 0;
    return i;
}
