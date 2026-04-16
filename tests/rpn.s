executable section "text"
    call serial_atoi
    bz a0, end ; exit on error
    mov t1, a1

    call serial_atoi
    bz a0, end ; exit on error
    mov t2, a1
    ; t1 <op> t2
wait_input_op:
    serial_get@ServiceTable
    spin
    bz a0, wait_input_op
    ; a1 contains op (1 char)    
    mov a1, t3

    seqi t0, t3, '+'
    bz t0, add
    add t1, t1, t2, zr
add:
    seqi t0, t3, '-'
    bz t0, sub
    sub t1, t1, t2, zr
sub:
    seqi t0, t3, '*'
    bz t0, mul
    mul t1, t1, t2, zr
mul:
    seqi t0, t3, '/'
    idiv t1, t1, t2, zr
    bz t0, div
div:
    ; t1 has result.
end:
    mov a0, t1 ; exit code is set to the result
    ret
    
serial_atoi: ; serial_atoi(): ?u32
    li t0, 0
wait_input:
    serial_get@ServiceTable ; serial_get(): ?u32
    ; assuming serial_get returns
    ; a0 -- 1: some, 0: none
    ; a1 -- value
    spin
    bz a0, wait_input

    seqi t1, a1, '\0' ; assume serial string is null terminated
    bn done
    
    sulti t1, a1, '0' ; char < '0'
    bn error

    sugti t1, a1, '9' ; char > '9'
    bn error

    subi a1, a1, '0'  ; a1 -= 48
    muli t0, t0, 10   ; t0 *= 10
    add t0, t0, a1, zr
    bz zr, wait_input
    li a0, 1 ; some
error:
    li a0, 0 ; none/fail
done:
    mov a1, t0 ; char
    ret
