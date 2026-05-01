strcmp(a, b) {
    auto i;
    i = 0;
    while (1) {
        if (a[i] != b[i]) {
            if (a[i] < b[i]) return -1;
            return 1;
        }
        if (a[i] == 0) return 0;
        i = i + 1;
    }
}
