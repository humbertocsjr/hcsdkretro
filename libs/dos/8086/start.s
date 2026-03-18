section text
    global _start
    _start:
        call _main
        int 0x20
        ret