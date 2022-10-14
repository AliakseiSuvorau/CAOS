.text

.global main

.extern getchar_unlocked
.extern putchar_unlocked

main:
	str x30, [sp, #-16]!
loop_begin:
	bl getchar_unlocked

	cmp x0,#-1
	beq loop_end

	cmp x0,#48 //0 ascii
	blt loop_begin
	cmp x0,#57 //9 ascii
	bgt loop_begin

	bl putchar_unlocked
	b loop_begin
loop_end:
	ldr x30, [sp], #16
	ret
