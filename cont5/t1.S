	.intel_syntax noprefix
	.text
	.global sum

sum:
	push rbx
	mov rax, rsi // eax = A
	mov rbx, rdx // ebx = B
	mov rdx, rcx // edx = R
	mov ecx, edi // ecx = N
	cmp ecx, 0
	je LoopEnd
LoopBegin:
	mov esi, [rax] // rsi = A[i]
	mov edi, [rbx] // rdi = B[i]

	mov [rdx], esi
	add [rdx], edi

	add rax, 4
	add rbx, 4
	add rdx, 4

	loop LoopBegin
LoopEnd:
	pop rbx
	ret
