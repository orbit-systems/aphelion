executable writable section "text"
add t1, zr, 5     ; t1 = 5
add t2, t1, t1, 5 ; t2 = t1 + t1 + 5 = 15
uremi t3, t2, 5   ; t3 = 3
