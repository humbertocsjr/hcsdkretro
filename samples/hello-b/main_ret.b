extrn putchar;
add3(a, b, c) {
    return a + b + c;
}
main() {
    auto x;
    x = add3(10, 20, 30);
    if (x == 60) putchar('Y');
    else putchar('N');
    putchar('0' + (x / 10));
    putchar('0' + (x % 10));
}
