	.intel_syntax noprefix

	.global dot_product

	.text
dot_product:
	// N -> rdi, A* -> rsi, B* -> rdx
	vxorpd ymm2, ymm2, ymm2
	mov rcx, rdi // счетчик цикла
	xor rax, rax // зануляем счетчик для адресов
while_begin:
	cmp rcx, 8 // проверяем, можем ли мы за раз взять 8 чисел из массива
	jl while_end

	vmovupd ymm0, [rsi + 4 * rax] // ymm0 = A[i]
	vmovupd ymm1, [rdx + 4 * rax] // ymm1 = B[i]
	add rax, 8 // увеличиваем счетчик

	vdpps ymm0, ymm0, ymm1, 0b11110001 // записывает скалярное произведение векторов из ymm1 и ymm2 в ymm1 по маске: берет 4 координаты в каждой половине регистров, ищет их скалярное произведение и кладет ответ в последнюю четверть половины регистра 
	vextractf128 xmm1, ymm0, 1 // достаем вторую часть из регистра ymm1 и записываем ее в xmm2
	vaddss xmm2, xmm2, xmm0 // прибавили к сумме произведение i-х координат
	vaddss xmm2, xmm2, xmm1

	sub rcx, 7 // так как надо уменьшить на 8
	loop while_begin
while_end:
	cmp rcx, 0
	je while2_end
while2_begin:
	vmovss xmm0, [rsi + 4 * rax]
	vmovss xmm1, [rdx + 4 * rax]
	inc rax

	mulss xmm0, xmm1
	addss xmm2, xmm0
	loop while2_begin
while2_end:
	movss xmm0, xmm2
	ret
