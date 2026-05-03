/* B test — arrays and strings */
extrn putchar;
buf[10];
main() {
    auto i;

    buf[0] = 65;  /* 'A' */
    buf[1] = 66;  /* 'B' */
    buf[2] = 67;  /* 'C' */
    i = 0;
    while (i < 3) {
        putchar(buf[i]);
        i = i + 1;
    }

    buf[5] = 88;  /* 'X' */
    putchar(buf[5]);

    putchar('!');
}
