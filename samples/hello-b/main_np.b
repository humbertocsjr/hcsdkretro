extrn putchar;
_print_u(n) {
    auto buf[10], i;
    i = 0;
    if (n == 0) { buf[0] = '0'; i = 1; }
    else { while (n > 0) { buf[i] = '0' + (n % 10); n = n / 10; i = i + 1; } }
    while (i > 0) { i = i - 1; putchar(buf[i]); }
}
main() {
    _print_u(42);
    putchar('!');
}
