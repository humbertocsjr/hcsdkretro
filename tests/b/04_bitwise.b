/* B test — bitwise, shift, logical not */
extrn putchar;
main() {
    auto a, b;
    a = 0xFF;
    b = 0x0F;

    if ((a & b) == 0x0F) putchar('A'); else putchar('N');
    if ((a | b) == 0xFF) putchar('O'); else putchar('N');
    if ((a ^ b) != 0)    putchar('X'); else putchar('N');
    if ((~a & 0x00FF) == 0) putchar('N'); else putchar('N');
    if ((a << 2) == 0x03FC) putchar('L'); else putchar('N');
    if ((b >> 1) == 7)   putchar('R'); else putchar('N');
}
