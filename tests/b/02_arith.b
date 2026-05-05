/* B test - arithmetic operators */
extrn putchar;
main() {
    auto a, b, r;
    a = 10;
    b = 3;

    r = a + b;
    if (r == 13) putchar('Y'); else putchar('N');

    r = a - b;
    if (r == 7) putchar('Y'); else putchar('N');

    r = 2 * 3;
    if (r == 6) putchar('Y'); else putchar('N');

    r = a / b;
    if (r == 3) putchar('Y'); else putchar('N');

    r = a % b;
    if (r == 1) putchar('Y'); else putchar('N');

    r = -5;
    if (r + 5 == 0) putchar('Y'); else putchar('N');
}
