//  int n = 20; // количество слагаемых
//  double arg = 0.;
//  std::cin >> arg;
//  double arg_i = arg;
//  double fact = 1;
//  double sum = 0;
//  for (int i = 1; i < n; ++i) {
//    double slag = 0;
//    arg_i *= arg; // i-я степень аргумента
//    fact *= i; // факториал
//    if (i % 2 == 1) {
//      slag += arg_i;
//      slag /= fact;
//      if ((i - 1) / 2 % 2 == 1) {
//        slag *= -1;
//      }
//    }
//    sum += slag;
//  }

	.intel_syntax noprefix
	.global avx_sin

	.text
avx_sin:
	enter 0, 0 // xmm0 = arg
	push rdi

	vxorpd ymm1, ymm1, ymm1 // ymm1 = sum
	mov rax, 1
	vxorpd ymm2, ymm2, ymm2
	cvtsi2sd xmm2, rax // ymm2 = arg_i
	mov rax, 1
	vxorpd ymm3, ymm3, ymm3
	cvtsi2sd xmm3, rax // ymm3 = fact
	xor rbx, rbx
	mov rbx, 1
while_begin:
	cmp rbx, 100
	je while_end

	inc rbx // счетчик цикла

	// считаем факториал
	vxorpd ymm5, ymm5, ymm5
	mov rax, rbx
	dec rax
	cvtsi2sd xmm5, rax
	vmulpd ymm3, ymm3, ymm5

	vxorpd ymm4, ymm4, ymm4 // ymm4 = slag
	vmulpd ymm2, ymm2, ymm0

	mov rax, rbx
	dec rax
	and rax, 1 // проверим остаток от деления
	cmp rax, 0
	je endif
	vaddpd ymm4, ymm4, ymm2
	vdivpd ymm4, ymm4, ymm3

	mov rax, rbx
	dec rax // так как цикл больше на 1
	dec rax
	shr rax, 1
	and rax, 1

	cmp rax, 0
	je endif

	vsubpd ymm1, ymm1, ymm4
	jmp while_begin
endif:
	vaddpd ymm1, ymm1, ymm4
	jmp while_begin
while_end:
	vmovapd ymm0, ymm1
	leave
	ret
