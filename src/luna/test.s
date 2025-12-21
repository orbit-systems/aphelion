executable section "text"

global start:
    mov l0, lr
    call lr, increment
    mov lr, l0
    li t0, mydata
    lw a0, [t0]
    ret

increment:
    li t0, mydata
    lw t1, [t0]
    addi t1, t1, 1
    sw [t0], ti
    ret

writable section "data"
mydata:
    d64 0xBAAD_F00D_DEAD_BEEF
