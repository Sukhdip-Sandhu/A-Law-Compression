	.file	"a_law_compression.c"
	.text
	.comm	inputfile,8,8
	.comm	outputfile,8,8
	.comm	input_file_name,8,8
	.comm	output_file_name,8,8
	.section	.rodata
.LC0:
	.string	"Error in malloc"
.LC1:
	.string	"No input file specified!"
.LC2:
	.string	"rb+"
.LC3:
	.string	"Error opening input file!"
.LC4:
	.string	"wb"
.LC5:
	.string	"Error writing output file!"
	.text
	.globl	main
	.type	main, @function
main:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$1128, %rsp
	.cfi_offset 3, -24
	movl	%edi, -1124(%rbp)
	movq	%rsi, -1136(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -24(%rbp)
	xorl	%eax, %eax
	movl	$1024, %edi
	call	malloc@PLT
	movq	%rax, input_file_name(%rip)
	movl	$1024, %edi
	call	malloc@PLT
	movq	%rax, output_file_name(%rip)
	movq	input_file_name(%rip), %rax
	testq	%rax, %rax
	jne	.L2
	leaq	.LC0(%rip), %rdi
	call	puts@PLT
	movl	$1, %edi
	call	exit@PLT
.L2:
	leaq	-1056(%rbp), %rax
	movl	$1024, %esi
	movq	%rax, %rdi
	call	getcwd@PLT
	testq	%rax, %rax
	je	.L3
	movq	input_file_name(%rip), %rax
	leaq	-1056(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	cmpl	$1, -1124(%rbp)
	jg	.L4
	leaq	.LC1(%rip), %rdi
	call	puts@PLT
	movl	$1, %edi
	call	exit@PLT
.L4:
	movq	input_file_name(%rip), %rdx
	movq	%rdx, %rax
	movq	$-1, %rcx
	movq	%rax, %rsi
	movl	$0, %eax
	movq	%rsi, %rdi
	repnz scasb
	movq	%rcx, %rax
	notq	%rax
	subq	$1, %rax
	addq	%rdx, %rax
	movw	$47, (%rax)
	movq	-1136(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rdx
	movq	input_file_name(%rip), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcat@PLT
.L3:
	movq	-1136(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rdx
	movq	output_file_name(%rip), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	strcpy@PLT
	movq	output_file_name(%rip), %rbx
	movq	output_file_name(%rip), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	subq	$4, %rax
	addq	%rbx, %rax
	movb	$0, (%rax)
	movq	output_file_name(%rip), %rdx
	movq	%rdx, %rax
	movq	$-1, %rcx
	movq	%rax, %rsi
	movl	$0, %eax
	movq	%rsi, %rdi
	repnz scasb
	movq	%rcx, %rax
	notq	%rax
	subq	$1, %rax
	addq	%rdx, %rax
	movabsq	$8530230210554852447, %rbx
	movq	%rbx, (%rax)
	movb	$0, 8(%rax)
	movq	input_file_name(%rip), %rax
	leaq	.LC2(%rip), %rsi
	movq	%rax, %rdi
	call	fopen@PLT
	movq	%rax, inputfile(%rip)
	movq	inputfile(%rip), %rax
	testq	%rax, %rax
	jne	.L5
	leaq	.LC3(%rip), %rdi
	call	puts@PLT
	movl	$1, %edi
	call	exit@PLT
.L5:
	movq	output_file_name(%rip), %rax
	leaq	.LC4(%rip), %rsi
	movq	%rax, %rdi
	call	fopen@PLT
	movq	%rax, outputfile(%rip)
	movq	outputfile(%rip), %rax
	testq	%rax, %rax
	jne	.L6
	leaq	.LC5(%rip), %rdi
	call	puts@PLT
	movl	$1, %edi
	call	exit@PLT
.L6:
	movq	inputfile(%rip), %rdx
	leaq	-1104(%rbp), %rax
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$44, %esi
	movq	%rax, %rdi
	call	fread@PLT
	movq	outputfile(%rip), %rdx
	leaq	-1104(%rbp), %rax
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$44, %esi
	movq	%rax, %rdi
	call	fwrite@PLT
	jmp	.L7
.L8:
	leaq	-1106(%rbp), %rax
	movq	%rax, %rdi
	call	bytes_to_int16
	cwtl
	movl	%eax, -1112(%rbp)
	movl	-1112(%rbp), %eax
	cwtl
	movl	%eax, %edi
	call	a_law_encode
	movb	%al, -1113(%rbp)
	movq	outputfile(%rip), %rdx
	leaq	-1113(%rbp), %rax
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	fwrite@PLT
.L7:
	movq	inputfile(%rip), %rdx
	leaq	-1106(%rbp), %rax
	movq	%rdx, %rcx
	movl	$2, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	fread@PLT
	cmpq	$2, %rax
	je	.L8
	movq	inputfile(%rip), %rax
	movq	%rax, %rdi
	call	fclose@PLT
	movq	outputfile(%rip), %rax
	movq	%rax, %rdi
	call	fclose@PLT
	movq	input_file_name(%rip), %rax
	movq	%rax, %rdi
	call	free@PLT
	movq	output_file_name(%rip), %rax
	movq	%rax, %rdi
	call	free@PLT
	movl	$0, %eax
	movq	-24(%rbp), %rbx
	xorq	%fs:40, %rbx
	je	.L10
	call	__stack_chk_fail@PLT
.L10:
	addq	$1128, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	main, .-main
	.globl	bytes_to_int16
	.type	bytes_to_int16, @function
bytes_to_int16:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movzbl	(%rax), %eax
	movb	%al, -4(%rbp)
	movq	-24(%rbp), %rax
	movzbl	1(%rax), %eax
	movb	%al, -3(%rbp)
	movzbl	-3(%rbp), %eax
	sall	$8, %eax
	movl	%eax, %edx
	movzbl	-4(%rbp), %eax
	orl	%edx, %eax
	movw	%ax, -2(%rbp)
	movzwl	-2(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	bytes_to_int16, .-bytes_to_int16
	.globl	a_law_encode
	.type	a_law_encode, @function
a_law_encode:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %eax
	movw	%ax, -20(%rbp)
	movw	$2048, -2(%rbp)
	movb	$0, -5(%rbp)
	movb	$11, -4(%rbp)
	movb	$0, -3(%rbp)
	cmpw	$0, -20(%rbp)
	jns	.L14
	movzwl	-20(%rbp), %eax
	negl	%eax
	movw	%ax, -20(%rbp)
	movb	$-128, -5(%rbp)
.L14:
	cmpw	$4095, -20(%rbp)
	jle	.L16
	movw	$4095, -20(%rbp)
	jmp	.L16
.L18:
	shrw	-2(%rbp)
	movzbl	-4(%rbp), %eax
	subl	$1, %eax
	movb	%al, -4(%rbp)
.L16:
	movswl	-20(%rbp), %edx
	movzwl	-2(%rbp), %eax
	andl	%eax, %edx
	movzwl	-2(%rbp), %eax
	cmpl	%eax, %edx
	je	.L17
	cmpb	$4, -4(%rbp)
	ja	.L18
.L17:
	movswl	-20(%rbp), %edx
	cmpb	$4, -4(%rbp)
	je	.L19
	movzbl	-4(%rbp), %eax
	subl	$4, %eax
	jmp	.L20
.L19:
	movl	$1, %eax
.L20:
	movl	%eax, %ecx
	sarl	%cl, %edx
	movl	%edx, %eax
	andl	$15, %eax
	movb	%al, -3(%rbp)
	movzbl	-4(%rbp), %eax
	subl	$4, %eax
	sall	$4, %eax
	movl	%eax, %edx
	movzbl	-5(%rbp), %eax
	orl	%eax, %edx
	movzbl	-3(%rbp), %eax
	orl	%edx, %eax
	xorl	$85, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	a_law_encode, .-a_law_encode
	.ident	"GCC: (Ubuntu 7.3.0-27ubuntu1~18.04) 7.3.0"
	.section	.note.GNU-stack,"",@progbits
