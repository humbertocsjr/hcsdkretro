section text
    global _start
    _start:
        ld sp, stack_top
        call _main
        ld c, 0
        call 5
        ret
section bss
    resb 1024
    stack_top: