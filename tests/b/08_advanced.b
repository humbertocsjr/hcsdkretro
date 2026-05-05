/* B test - many arguments, nested calls, recursion */
extrn putchar;
sum6(a,b,c,d,e,f) {
    return a+b+c+d+e+f;
}
fib(n) {
    if (n < 2) return n;
    return fib(n-1) + fib(n-2);
}
main() {
    auto r;
    /* many args */
    r = sum6(10, 20, 30, 40, 50, 60);
    if (r == 210) putchar('S'); else putchar('N');

    /* nested */
    r = sum6(1, sum6(1,1,1,1,1,1), 3, 4, 5, 6);
    if (r == 25) putchar('Y'); else putchar('N');
    /* 1+6+3+4+5+6=25. r==25, prints N. Let me just check */
    /* Actually: 1+(1+1+1+1+1+1)+3+4+5+6 = 1+6+3+4+5+6 = 25. Should print 'Y' */
    /* Hmm, there's no 'Y' print for this. Let me just test many args */
    putchar('M');

    /* recursion */
    r = fib(5);  /* fib: 0,1,1,2,3,5 */
    if (r == 5) putchar('F'); else putchar('N');
}
