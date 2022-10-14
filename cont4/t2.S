	.text

	.global main

	.extern printf
	.extern scanf
main:
	str x30, [sp, #-16]!
	ldr x0,=input
	ldr x1,=num1
	ldr x2,=num2
	bl scanf
	ldr x1,=num1
	ldr x2,=num2
	ldr x1,[x1]
	ldr x2,[x2]

	add x1,x1,x2

	ldr x0,=output
	bl printf

	ldr x30, [sp], #16
	ret

	.data
output:
	.asciz "%d\n"
input:
	.ascii "%d %d"
num1:
	.word 0
num2:
	.word 0
