executable writable section "text" align 4
    global label1:
        sub l0, l0, zr, mystring.end - mystring
        bz l1, label2
        add a0, a0, a1

    label2:
        add a0, a0, a1
        bz l1, label1

section "data"
    mystring:
        string "hello!"
    mystring.end:
