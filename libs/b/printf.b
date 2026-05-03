extrn putchar;
_print_digit(d) {
    if (d >= 10) putchar(d - 10 + 'a');
    else putchar(d + '0');
}
_print_u(n) {
    auto buf[10], i;
    i = 0;
    if (n == 0) { buf[0] = '0'; i = 1; }
    else { while (n > 0) { buf[i] = '0' + (n % 10); n = n / 10; i = i + 1; } }
    while (i > 0) { i = i - 1; putchar(buf[i]); }
}
_print_s(s) {
    auto i, ch;
    i = 0;
    ch = peekb(s + i);
    while (ch != 0) { putchar(ch); i = i + 1; ch = peekb(s + i); }
}
_print_x(n) {
    auto i, sh, nb;
    i = 0;
    while (i < 4) {
        sh = 12 - i * 4; nb = n >> sh; nb = nb & 0x0F;
        _print_digit(nb);
        i = i + 1;
    }
}
printf(fmt, a, b, c, d, e, f, g, h) {
    auto i, ch, arg;
    i = 0; arg = a;
    ch = peekb(fmt + i);
    while (ch != 0) {
        if (ch == '%') {
            i = i + 1; ch = peekb(fmt + i);
            if (ch == 'd') {
                if (arg & 0x8000) { putchar('-'); _print_u(0 - arg); }
                else _print_u(arg);
                arg = arg + 1;
            } else if (ch == 'u') { _print_u(arg); arg = arg + 1; }
            else if (ch == 'x') { _print_x(arg); arg = arg + 1; }
            else if (ch == 's') { _print_s(arg); arg = arg + 1; }
            else if (ch == 'c') { putchar(arg); arg = arg + 1; }
            else if (ch == '%') putchar('%');
            i = i + 1;
        } else { putchar(ch); i = i + 1; }
        ch = peekb(fmt + i);
    }
}
