	.intel_syntax noprefix
	.global very_important_function

	.text
very_important_function: // (size_t N, float* A, float* B, float* R)
//                                rdi,      rsi,      rdx,      rcx
	enter 0, 0
	push rdi

	xor rax, rax // rax = 0
	xorps xmm0, xmm0 // xmm0 = 0 (packed single)
	xorps xmm1, xmm1 // xmm1 = 0
.Lwhile:
	cmp rdi, rax
	je .Lexit

	movss xmm1, dword ptr [rsi + rax * 4]
	addss xmm1, dword ptr [rdx + rax * 4] //считаем сумму i-х чисел
	addss xmm0, xmm1 // считаем сумму всех сумм
	movss dword ptr [rcx + rax * 4], xmm1 // сохраняем результат в R[i]
	inc rax
	jmp .Lwhile
.Lexit:
	cvtss2sd xmm1, xmm0 // даем сумме сумм двойную точность (double) и перемещаем сумму сумм в xmm1
	movsd xmm0, xmm1    // возвращаем с удоенной точностью
	cvtsi2sd xmm1, rdi
	divsd xmm0, xmm1

	leave
	ret
