/* B test — comparison and logical operators */
extrn putchar;
main() {
    auto a, b;
    a = 10;
    b = 3;

    if (a > b)  putchar('>'); else putchar('X');
    if (a < 100) putchar('<'); else putchar('X');
    if (a >= 10) putchar('G'); else putchar('X');
    if (b <= 3)  putchar('L'); else putchar('X');
    if (a == 10) putchar('E'); else putchar('X');
    if (a != b)  putchar('N'); else putchar('X');

    if (a && b)  putchar('A'); else putchar('X');
    if (!b)      putchar('X'); else putchar('B');
    if (a || 0)  putchar('O'); else putchar('X');
}
