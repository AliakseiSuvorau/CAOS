	.intel_syntax noprefix

	.global _start

_start:
	sub rsp, 16
	xor rax, rax
	mov rdi, 0
	mov rsi, rsp
	mov rdx, 1
	mov rax, 0  // SYS_read
	syscall

cycle_begin:
	cmp rax, 0
		je cycle_end

	mov rdi, 1
	mov rax, 1  // SYS_write
	syscall

	mov rdi, 0
	xor rdi, rdi
	mov rax, 0  // SYS_read
	syscall
	jmp cycle_begin

cycle_end:
	add rsp, 16
	xor rdi, rdi
	mov rax, 60  // SYS_ext
	syscall
