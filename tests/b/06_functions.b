/* B test — functions: calls, parameters, return values */
extrn putchar;
add(a, b) {
    return a + b;
}
is_zero(n) {
    if (n == 0) return 1;
    return 0;
}
main() {
    auto r;
    r = add(10, 20);
    if (r == 30) putchar('A'); else putchar('N');

    if (is_zero(0)) putchar('Z'); else putchar('N');
    if (!is_zero(5)) putchar('N'); else putchar('Z');

    r = add(add(1, 2), add(3, 4));
    if (r == 10) putchar('T'); else putchar('N');
}
