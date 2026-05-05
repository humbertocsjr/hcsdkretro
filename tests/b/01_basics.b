/* B test - basic types: auto, globals, extrn, chars, numbers */
extrn putchar;
g;
main() {
    auto a;
    a = 65;          /* 'A' */
    putchar(a);

    g = 66;          /* 'B' */
    putchar(g);

    putchar(67);     /* 'C' - direct char constant */

    a = 'D';         /* 'D' */
    putchar(a);

    putchar('!');    /* '!' */
}
