extrn ft;
setb(base, off, val) {
    auto w, o, t;
    w = off / 2; o = off % 2;
    t = *(base + w);
    if (o == 0) *(base + w) = (t & 0xFF00) | val;
    else *(base + w) = (t & 0x00FF) | (val << 8);
}
getb(base, off) {
    auto w, o, t;
    w = off / 2; o = off % 2;
    t = *(base + w);
    if (o == 0) return t & 0xFF;
    else return (t >> 8) & 0xFF;
}
