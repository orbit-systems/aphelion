executable writable section "text"
    ssi a0, 0, 0
    ssi a1, 1, 0
    ssi a2, 1, 0
label1:
    add a2, a0, a1
    add a0, a1, zr
    add a1, a2, zr
    bz zr, label1
