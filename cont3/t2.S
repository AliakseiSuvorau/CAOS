.text

.global sum

sum:
	mov x3,0
loop_start:
	cmp x3,x1
	bge loop_end

	ldr x4,[x2]
	add x0,x0,x4
	add x2,x2,4
	add x3,x3,1
	b loop_start
loop_end:
	ret
