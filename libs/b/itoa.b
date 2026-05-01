itoa(n, s) {
    auto i, neg, tmp, j, k;
    i = 0; neg = 0;
    if (n < 0) { neg = 1; n = 0 - n; }
    if (n == 0) { s[0] = '0'; i = 1; }
    else { while (n > 0) { s[i] = '0' + (n % 10); n = n / 10; i = i + 1; } }
    if (neg) { s[i] = '-'; i = i + 1; }
    s[i] = 0;
    j = 0; k = i - 1;
    if (neg) k = k - 1;
    while (j < k) { tmp = s[j]; s[j] = s[k]; s[k] = tmp; j = j + 1; k = k - 1; }
    return s;
}
