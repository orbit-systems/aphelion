section "data"
	message:
		string "Hello, world!\n"
	message_len:

executable section "text"
	li t0, message ; ptr
	li t1, 0 ; i 
	li t2, message_len - message - 1 ; string len
	
print_loop:
	lb t3, [t0 + t1] ; ptr + i
	addi t1, 1       ; i ++
	push t3
	fcall serial_put@ServiceTable
	sule t3, t1, t2
	bn t3, print_loop

	ret
