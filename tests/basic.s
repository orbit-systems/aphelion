executable writable section "text"
    call add7 + 4
    bz zr, add7
    bz zr, 0
    fcall doublePlus5
    nop
    nop
    li a1, doublePlus5
    call doublePlus5
add7:
    addi a0, a0, 0x4000 + 7 ; test 14 bit. Expected: 7
    ret
executable writable section "text"
doublePlus5:
    add a0, a0, a0, 0x200 + 5 ; test 9 bit. Expected: 5
    ret
