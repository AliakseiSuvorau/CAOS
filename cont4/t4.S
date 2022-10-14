.text

.global main

.extern realloc
.extern free

.extern getchar_unlocked
.extern putchar_unlocked

main:
	str x30, [sp, #-16]!
	str x19, [sp, #-16]!
        str x20, [sp, #-16]!
	mov x19, #0 //nullptr
	mov x20, #8 //size * 8
while:
	mov x0, x19
	mov x1, x20
	bl realloc
	mov x19, x0
	add x20, x20, #8 // выделяю память под массив и увеличиваю его размер на 1

	bl getchar_unlocked //в x0 записываем новую чиселку
	cmp x0, #-1
	beq while_end
	mov x2, x0 // считанная чиселка теперь в регистре х2

	add x0, x19, x20
	sub x0, x0, #16
	str x2, [x0] // добавляем чиселку в массив

	b while
while_end:
	sub x20, x20, #24 // тут теперь (размер массива - 1) * 8
while_out:
	cmp x20, #0
	blt while_out_end

	mov x2, x19
	add x2, x2, x20
	ldr x0, [x2]
	bl putchar_unlocked
	sub x20, x20, #8
	b while_out
while_out_end:
	mov x0, x19
	bl free

	ldr x20, [sp], #16
	ldr x19, [sp], #16
	ldr x30, [sp], #16
	ret
