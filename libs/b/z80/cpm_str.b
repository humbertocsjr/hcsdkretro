strc(s, n) {
    auto w, o;
    w = n / 2; o = n % 2;
    if (o == 0) return s[w] & 0xFF;
    else return (s[w] >> 8) & 0xFF;
}
toup(c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}
