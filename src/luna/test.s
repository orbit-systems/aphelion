execute section "text"
global start:
    call lr, increment
    li t0, mydata
    lw a0, [t0]
    ret

increment:
    li t0, mydata
    lw t1, [t0]
    addi t1, t1, 1
    sw [t0], ti
    ret

write section "data"
mydata:
    d64 0xBAAD_F00D_DEAD_BEEF
