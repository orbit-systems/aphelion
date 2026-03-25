executable writable section "text"
    li a0, 0
    li a1, 1
    li a2, 1
label1:
    add a2, a0, a1
    add a0, a1, zr
    add a1, a2, zr
    bz zr, -4
