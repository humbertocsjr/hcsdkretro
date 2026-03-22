section text
    global _start
    _start:
        mov sp, stack_top
        call _main
        int 0x20
        ret

section bss
    resb 1024
    stack_top: