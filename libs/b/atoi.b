atoi(s) {
    auto i, n, sign;
    i = 0; sign = 1;
    if (s[i] == '-') { sign = 0 - 1; i = i + 1; }
    else if (s[i] == '+') { i = i + 1; }
    n = 0;
    while (s[i] >= '0' && s[i] <= '9') { n = n * 10 + (s[i] - '0'); i = i + 1; }
    return n * sign;
}
