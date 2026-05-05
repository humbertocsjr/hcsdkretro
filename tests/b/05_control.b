/* B test - control flow: if/else, while, break */
extrn putchar;
main() {
    auto i;

    i = 0;
    while (i < 3) {
        putchar('A' + i);
        i = i + 1;
    }

    if (1) putchar('Y'); else putchar('N');
    if (0) putchar('N'); else putchar('E');

    i = 5;
    while (i) {
        i = i - 1;
        if (i == 2) break;
    }
    putchar('0' + i);
}
