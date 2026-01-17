executable section "text" align 8
label:
    add a0, a0, a1, mystring.end - mystring
    add a0, a0, a1
    add a0, a0, a1
    
label:
    add a0, a0, a1
    bz zr, label

align 4
mystring_len:
    hword mystring.end - mystring

mystring:
    string "hello!"
mystring.end:
