/* B test - compound assignment operators */
extrn putchar;
main() {
    auto a;

    a = 10; a += 5;
    if (a == 15) putchar('A'); else putchar('N');

    a -= 3;
    if (a == 12) putchar('S'); else putchar('N');

    a = 2; a *= 4;
    if (a == 8) putchar('M'); else putchar('N');

    a = 20; a /= 5;
    if (a == 4) putchar('D'); else putchar('N');

    a = 17; a %= 5;
    if (a == 2) putchar('P'); else putchar('N');

    a = 0x0F; a &= 0x03;
    if (a == 3) putchar('A'); else putchar('N');

    a = 0x0C; a |= 0x03;
    if (a == 0x0F) putchar('O'); else putchar('N');

    a = 0xFF; a ^= 0x0F;
    if (a == 0xF0) putchar('X'); else putchar('N');
}
