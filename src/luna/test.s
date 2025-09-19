executable 
section "text"
align 4
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

get_first_16:
    ssi.c a0, noerror q0 mydata, 0
    ret

writable 
section "data"
align 8
mydata:
    d64 0xBAAD_F00D_DEAD_BEEF
