.data
	.align 4
	.type	 PACKED_0,@object
	.size	 PACKED_0,8
PACKED_0:
	.long 0
	.long 0
	.align 4
	.type	 PACKED_1,@object
	.size	 PACKED_1,8
PACKED_1:
	.long 65537
	.long 65537
	.align 4
	.type	 PACKED_2,@object
	.size	 PACKED_2,8
PACKED_2:
	.long 131074
	.long 131074
	.align 4
	.type	 MMX_MASK_1,@object
	.size	 MMX_MASK_1,8
MMX_MASK_1:
	.long 16843009
	.long 16843009
	.align 4
	.type	 MMX_MASK_2,@object
	.size	 MMX_MASK_2,8
MMX_MASK_2:
	.long -16843010
	.long -16843010
.text
	.align 16

.globl pred_comp_mmx
	.type	 pred_comp_mmx,@function
pred_comp_mmx:
	subl $36,%esp
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx
	call .L222
.L222:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L222],%ebx
	movl 80(%esp),%ecx
	movl 84(%esp),%esi
	movl 88(%esp),%eax
	movl %esi,16(%esp)
	sarl $1,16(%esp)
	movl %eax,20(%esp)
	andl $1,20(%esp)
	sarl $1,%eax
	addl %ecx,%eax
	imull 64(%esp),%eax
	addl 56(%esp),%eax
	movl 16(%esp),%edx
	addl 76(%esp),%edx
	leal (%edx,%eax),%ebp
	imull 64(%esp),%ecx
	addl 60(%esp),%ecx
	addl 76(%esp),%ecx
	testl $1,%esi
	jne .L220
	cmpl $0,20(%esp)
	jne .L105
	cmpl $0,92(%esp)
	je .L106

	movq MMX_MASK_1@GOTOFF(%ebx), %mm5
	movq MMX_MASK_2@GOTOFF(%ebx), %mm4
	cmpl $8,68(%esp)
	jne .L107

	movl 72(%esp),%esi
	movl 64(%esp),%eax
.L111:
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)

	addl %eax,%ebp
	addl %eax,%ecx
	decl %esi
	jnz .L111

	jmp .L131
.L107:
	movl 72(%esp),%esi
	movl 64(%esp),%eax
.L118:
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
	movq 8(%ebp), %mm1

	movq %mm1, %mm3
	movq 8(%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, 8(%ecx)
	addl %eax,%ebp
	addl %eax,%ecx
	decl %esi
	jnz .L118

	jmp .L131
.L106:
	xorl %esi,%esi
	cmpl 72(%esp),%esi
	jge .L131
	.p2align 4,,7
.L124:
	movl $0,16(%esp)
	movl 68(%esp),%edi
	cmpl %edi,16(%esp)
	jge .L126
	.p2align 4,,7
.L128:
	movl 16(%esp),%edx
	movb (%edx,%ebp),%al
	movb %al,(%edx,%ecx)
	incl %edx
	movl %edx,16(%esp)
	movl 68(%esp),%edi
	cmpl %edi,%edx
	jl .L128
.L126:
	addl 64(%esp),%ebp
	addl 64(%esp),%ecx
	incl %esi
	cmpl 72(%esp),%esi
	jl .L124
	jmp .L131
	.p2align 4,,7
.L105:
	cmpl $0,92(%esp)
	je .L133
#APP
	movq MMX_MASK_1@GOTOFF(%ebx), %mm5
	movq MMX_MASK_2@GOTOFF(%ebx), %mm4
#NO_APP
	cmpl $8,68(%esp)
	jne .L134
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	.p2align 4,,7
.L138:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 64(%esp),%edx
#APP
	movq (%edx,%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
#NO_APP
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L138
	jmp .L131
	.p2align 4,,7
.L134:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal 8(%ecx),%edi
	movl %edi,16(%esp)
	leal 8(%ebp),%eax
	.p2align 4,,7
.L145:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 64(%esp),%edx
#APP
	movq (%edx,%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
	movq (%eax), %mm1
	movq %mm1, %mm3
	movq (%edx,%eax), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
#NO_APP
	movl 16(%esp),%edi
#APP
	movq (%edi), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%edi)
#NO_APP
	addl %edx,%eax
	addl %edx,%ebp
	addl %edx,%edi
	movl %edi,16(%esp)
	addl %edx,%ecx
	decl %esi
	jnz .L145
	jmp .L131
	.p2align 4,,7
.L133:
#APP
	movq MMX_MASK_1@GOTOFF(%ebx), %mm5
	movq MMX_MASK_2@GOTOFF(%ebx), %mm4
#NO_APP
	cmpl $8,68(%esp)
	jne .L148
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	.p2align 4,,7
.L152:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 64(%esp),%edx
#APP
	movq (%edx,%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
#NO_APP
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L152
	jmp .L131
	.p2align 4,,7
.L148:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal 8(%ebp),%eax
	.p2align 4,,7
.L159:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 64(%esp),%edi
#APP
	movq (%edi,%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
	movq (%eax), %mm1
	movq %mm1, %mm3
	movq (%edi,%eax), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, 8(%ecx)
#NO_APP
	addl %edi,%eax
	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L159
	jmp .L131
	.p2align 4,,7
.L220:
	cmpl $0,20(%esp)
	jne .L162
	cmpl $0,92(%esp)
	je .L163
#APP
	movq MMX_MASK_1@GOTOFF(%ebx), %mm5
	movq MMX_MASK_2@GOTOFF(%ebx), %mm4
#NO_APP
	cmpl $8,68(%esp)
	jne .L164
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	.p2align 4,,7
.L168:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq 1(%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
#NO_APP
	addl 64(%esp),%ebp
	addl 64(%esp),%ecx
	decl %esi
	jnz .L168
	jmp .L131
	.p2align 4,,7
.L164:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal 8(%ecx),%edx
	movl %edx,16(%esp)
	leal 9(%ebp),%edi
	movl %edi,20(%esp)
	leal 8(%ebp),%eax
	.p2align 4,,7
.L175:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq -7(%eax), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
	movq (%ecx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
	movq (%eax), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 20(%esp),%edx
#APP
	movq (%edx), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, %mm3
#NO_APP
	movl 16(%esp),%edi
#APP
	movq (%edi), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%edi)
#NO_APP
	addl 64(%esp),%eax
	movl 64(%esp),%edi
	addl %edi,%edx
	movl %edx,20(%esp)
	addl %edi,%ebp
	addl %edi,16(%esp)
	addl %edi,%ecx
	decl %esi
	jnz .L175
	jmp .L131
	.p2align 4,,7
.L163:
#APP
	movq MMX_MASK_1@GOTOFF(%ebx), %mm5
	movq MMX_MASK_2@GOTOFF(%ebx), %mm4
#NO_APP
	cmpl $8,68(%esp)
	jne .L178
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	.p2align 4,,7
.L182:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq 1(%ebp), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
#NO_APP
	addl 64(%esp),%ebp
	addl 64(%esp),%ecx
	decl %esi
	jnz .L182
	jmp .L131
	.p2align 4,,7
.L178:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal 9(%ebp),%edx
	movl %edx,16(%esp)
	leal 8(%ebp),%eax
	.p2align 4,,7
.L189:
#APP
	movq (%ebp), %mm1
	movq %mm1, %mm3
	movq -7(%eax), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, (%ecx)
	movq (%eax), %mm1
	movq %mm1, %mm3
#NO_APP
	movl 16(%esp),%edi
#APP
	movq (%edi), %mm2
	por %mm2, %mm3
	pand %mm5, %mm3
	pand %mm4, %mm1
	pand %mm4, %mm2
	psrlq $1, %mm1
	psrlq $1, %mm2
	paddusb %mm2, %mm1
	paddusb %mm3, %mm1
	movq %mm1, 8(%ecx)
#NO_APP
	addl 64(%esp),%eax
	movl 64(%esp),%edx
	addl %edx,%edi
	movl %edi,16(%esp)
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L189
	jmp .L131
	.p2align 4,,7
.L162:
	cmpl $0,92(%esp)
	je .L192
	cmpl $8,68(%esp)
	jne .L193
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal PACKED_0@GOTOFF(%ebx),%eax
	leal PACKED_2@GOTOFF(%ebx),%edi
	movl %edi,48(%esp)
	leal PACKED_1@GOTOFF(%ebx),%edx
	movl %edx,20(%esp)
	leal 1(%ebp),%edi
	movl %edi,16(%esp)
	.p2align 4,,7
.L197:
#APP
	movq (%ebp), %mm0
	movq %mm0, %mm4
#NO_APP
	movl 16(%esp),%edx
#APP
	movq (%edx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 64(%esp),%edi
#APP
	movq (%edi,%ebp), %mm2
	movq %mm2, %mm6
	movq (%edi,%edx), %mm3
	movq %mm3, %mm7
	punpcklbw (%eax), %mm0
	punpcklbw (%eax), %mm1
	punpcklbw (%eax), %mm2
	punpcklbw (%eax), %mm3
	punpckhbw (%eax), %mm4
	punpckhbw (%eax), %mm5
	punpckhbw (%eax), %mm6
	punpckhbw (%eax), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
	movq (%ecx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 48(%esp),%edx
#APP
	paddw (%edx), %mm0
	paddw (%edx), %mm4
	punpcklbw (%eax), %mm1
	punpckhbw (%eax), %mm5
	psrlw $2, %mm0
	psrlw $2, %mm4
#NO_APP
	movl 20(%esp),%edi
#APP
	paddw (%edi), %mm0
	paddw (%edi), %mm4
	paddw %mm1, %mm0
	paddw %mm5, %mm4
	psrlw $1, %mm0
	psrlw $1, %mm4
	packuswb %mm4, %mm0
	movq %mm0, (%ecx)
#NO_APP
	movl 64(%esp),%edx
	addl %edx,16(%esp)
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L197
	jmp .L131
	.p2align 4,,7
.L193:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal PACKED_0@GOTOFF(%ebx),%eax
	leal PACKED_2@GOTOFF(%ebx),%edi
	movl %edi,44(%esp)
	leal PACKED_1@GOTOFF(%ebx),%edx
	movl %edx,40(%esp)
	leal 8(%ecx),%edi
	movl %edi,32(%esp)
	leal 9(%ebp),%edx
	movl %edx,20(%esp)
	leal 8(%ebp),%edi
	movl %edi,16(%esp)
	.p2align 4,,7
.L204:
#APP
	movq (%ebp), %mm0
	movq %mm0, %mm4
#NO_APP
	movl 16(%esp),%edx
#APP
	movq -7(%edx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 64(%esp),%edi
#APP
	movq (%edi,%ebp), %mm2
	movq %mm2, %mm6
	movq 1(%edi,%ebp), %mm3
	movq %mm3, %mm7
	punpcklbw (%eax), %mm0
	punpcklbw (%eax), %mm1
	punpcklbw (%eax), %mm2
	punpcklbw (%eax), %mm3
	punpckhbw (%eax), %mm4
	punpckhbw (%eax), %mm5
	punpckhbw (%eax), %mm6
	punpckhbw (%eax), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
	movq (%ecx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 44(%esp),%edx
#APP
	paddw (%edx), %mm0
	paddw (%edx), %mm4
	punpcklbw (%eax), %mm1
	punpckhbw (%eax), %mm5
	psrlw $2, %mm0
	psrlw $2, %mm4
#NO_APP
	movl 40(%esp),%edi
#APP
	paddw (%edi), %mm0
	paddw (%edi), %mm4
	paddw %mm1, %mm0
	paddw %mm5, %mm4
	psrlw $1, %mm0
	psrlw $1, %mm4
	packuswb %mm4, %mm0
	movq %mm0, (%ecx)
#NO_APP
	movl 16(%esp),%edx
#APP
	movq (%edx), %mm0
	movq %mm0, %mm4
#NO_APP
	movl 20(%esp),%edi
#APP
	movq (%edi), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 64(%esp),%edx
#APP
	movq 8(%edx,%ebp), %mm2
	movq %mm2, %mm6
	movq (%edx,%edi), %mm3
	movq %mm3, %mm7
	punpcklbw (%eax), %mm0
	punpcklbw (%eax), %mm1
	punpcklbw (%eax), %mm2
	punpcklbw (%eax), %mm3
	punpckhbw (%eax), %mm4
	punpckhbw (%eax), %mm5
	punpckhbw (%eax), %mm6
	punpckhbw (%eax), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
#NO_APP
	movl 32(%esp),%edi
#APP
	movq (%edi), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 44(%esp),%edx
#APP
	paddw (%edx), %mm0
	paddw (%edx), %mm4
	punpcklbw (%eax), %mm1
	punpckhbw (%eax), %mm5
	psrlw $2, %mm0
	psrlw $2, %mm4
#NO_APP
	movl 40(%esp),%edi
#APP
	paddw (%edi), %mm0
	paddw (%edi), %mm4
	paddw %mm1, %mm0
	paddw %mm5, %mm4
	psrlw $1, %mm0
	psrlw $1, %mm4
	packuswb %mm4, %mm0
#NO_APP
	movl 32(%esp),%edx
#APP
	movq %mm0, (%edx)
#NO_APP
	movl 64(%esp),%edi
	addl %edi,16(%esp)
	addl %edi,20(%esp)
	addl %edi,%ebp
	addl %edi,%edx
	movl %edx,32(%esp)
	addl %edi,%ecx
	decl %esi
	jnz .L204
	jmp .L131
	.p2align 4,,7
.L192:
	cmpl $8,68(%esp)
	jne .L207
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal PACKED_0@GOTOFF(%ebx),%eax
	leal PACKED_2@GOTOFF(%ebx),%edx
	movl %edx,20(%esp)
	leal 1(%ebp),%edi
	movl %edi,16(%esp)
	.p2align 4,,7
.L211:
#APP
	movq (%ebp), %mm0
	movq %mm0, %mm4
#NO_APP
	movl 16(%esp),%edx
#APP
	movq (%edx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 64(%esp),%edi
#APP
	movq (%edi,%ebp), %mm2
	movq %mm2, %mm6
	movq (%edi,%edx), %mm3
	movq %mm3, %mm7
	punpcklbw (%eax), %mm0
	punpcklbw (%eax), %mm1
	punpcklbw (%eax), %mm2
	punpcklbw (%eax), %mm3
	punpckhbw (%eax), %mm4
	punpckhbw (%eax), %mm5
	punpckhbw (%eax), %mm6
	punpckhbw (%eax), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
#NO_APP
	movl 20(%esp),%edx
#APP
	paddw (%edx), %mm0
	paddw (%edx), %mm4
	psrlw $2, %mm0
	psrlw $2, %mm4
	packuswb %mm4, %mm0
	movq %mm0, (%ecx)
#NO_APP
	addl %edi,16(%esp)
	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L211
	jmp .L131
	.p2align 4,,7
.L207:
	cmpl $16,68(%esp)
	jne .L131
	movl 72(%esp),%esi
	testl %esi,%esi
	je .L131
	leal PACKED_0@GOTOFF(%ebx),%edi
	movl %edi,16(%esp)
	leal PACKED_2@GOTOFF(%ebx),%edx
	movl %edx,20(%esp)
	leal 9(%ebp),%edi
	movl %edi,36(%esp)
	leal 8(%ebp),%eax
	movl 64(%esp),%edx
	addl %eax,%edx
	movl %edx,28(%esp)
	movl 64(%esp),%edi
	leal 1(%edi,%ebp),%edi
	movl %edi,24(%esp)
	.p2align 4,,7
.L218:
#APP
	movq (%ebp), %mm0
	movq %mm0, %mm4
	movq -7(%eax), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 64(%esp),%edx
#APP
	movq (%edx,%ebp), %mm2
	movq %mm2, %mm6
#NO_APP
	movl 24(%esp),%edi
#APP
	movq (%edi), %mm3
	movq %mm3, %mm7
#NO_APP
	movl 16(%esp),%edx
#APP
	punpcklbw (%edx), %mm0
	punpcklbw (%edx), %mm1
	punpcklbw (%edx), %mm2
	punpcklbw (%edx), %mm3
	punpckhbw (%edx), %mm4
	punpckhbw (%edx), %mm5
	punpckhbw (%edx), %mm6
	punpckhbw (%edx), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
#NO_APP
	movl 20(%esp),%edi
#APP
	paddw (%edi), %mm0
	paddw (%edi), %mm4
	psrlw $2, %mm0
	psrlw $2, %mm4
	packuswb %mm4, %mm0
	movq %mm0, (%ecx)
	movq (%eax), %mm0
	movq %mm0, %mm4
#NO_APP
	movl 36(%esp),%edx
#APP
	movq (%edx), %mm1
	movq %mm1, %mm5
#NO_APP
	movl 28(%esp),%edi
#APP
	movq (%edi), %mm2
	movq %mm2, %mm6
#NO_APP
	movl 64(%esp),%edi
#APP
	movq (%edi,%edx), %mm3
	movq %mm3, %mm7
#NO_APP
	movl 16(%esp),%edx
#APP
	punpcklbw (%edx), %mm0
	punpcklbw (%edx), %mm1
	punpcklbw (%edx), %mm2
	punpcklbw (%edx), %mm3
	punpckhbw (%edx), %mm4
	punpckhbw (%edx), %mm5
	punpckhbw (%edx), %mm6
	punpckhbw (%edx), %mm7
	paddw %mm1, %mm0
	paddw %mm3, %mm2
	paddw %mm5, %mm4
	paddw %mm7, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm4
#NO_APP
	movl 20(%esp),%edi
#APP
	paddw (%edi), %mm0
	paddw (%edi), %mm4
	psrlw $2, %mm0
	psrlw $2, %mm4
	packuswb %mm4, %mm0
	movq %mm0, 8(%ecx)
#NO_APP
	addl 64(%esp),%eax
	movl 64(%esp),%edx
	addl %edx,36(%esp)
	addl %edx,28(%esp)
	addl %edx,24(%esp)
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L218
.L131:
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	addl $36,%esp
	ret
