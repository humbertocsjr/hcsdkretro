extrn ft;
find_slot() {
    auto i;
    i = 0;
    while (i < 5) {
        if (ft[i * 24 + 23] == 0) return i;
        i = i + 1;
    }
    return -1;
}
