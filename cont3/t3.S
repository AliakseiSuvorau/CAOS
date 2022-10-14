.text

.global solve

solve:
	sxtw x0,w0
	sxtw x1,w1
	sxtw x2,w2
	sxtw x3,w3
	mov x4,x3
	mov x3,x2
	mov x2,x1
	mov x1,x0
	mov x0,0
loop_begin:
	cmp x0,255
	bge loop_end

	mul x5,x1,x0
	add x5,x5,x2
        mul x5,x5,x0
        add x5,x5,x3
        mul x5,x5,x0
        add x5,x5,x4

	cmp x5,0
	beq loop_end
	add x0,x0,1
	b loop_begin
loop_end:
	ret
