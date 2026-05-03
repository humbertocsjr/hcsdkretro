global exit
exit:
    push b
    mvi c, 0
    call 5
    pop b
    ret
