executable writable section "text" align 8
    add a0, a0, a1, mystring.end - mystring
    add a0, a0, a1
    add a0, a0, a1
    bz l1, label
    add a0, a0, a1
    
global label:
    add a0, a0, a1
    bz l1, label

mystring:
    string "hello!"
mystring.end:
