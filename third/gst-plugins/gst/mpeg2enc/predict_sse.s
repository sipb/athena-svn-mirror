
.globl pred_comp_sse
	.type	 pred_comp_sse,@function
pred_comp_sse:
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

	cmpl $8,68(%esp)
	jne .L107

	movl 72(%esp),%esi
	movl 64(%esp),%eax
.L111:
	movq (%ebp), %mm1
	pavgb (%ecx), %mm1
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
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm1
	pavgb 8(%ecx), %mm1
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

	cmpl $8,68(%esp)
	jne .L134

	movl 72(%esp),%esi
	movl 64(%esp),%edx

	.p2align 4,,7
.L138:
	movq (%ebp), %mm1
	pavgb (%edx,%ebp), %mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L138

	jmp .L131

	.p2align 4,,7
.L134:
	movl 72(%esp),%esi
	movl 64(%esp),%edx

	.p2align 4,,7
.L145:
	movq (%ebp), %mm1
	pavgb (%edx,%ebp), %mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm1
	pavgb 8(%edx,%ebp), %mm1
	pavgb 8(%ecx), %mm1
	movq %mm1, 8(%ecx)

	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L145

	jmp .L131
	.p2align 4,,7
.L133:
	cmpl $8,68(%esp)
	jne .L148

	movl 72(%esp),%esi
	movl 64(%esp),%edx

	.p2align 4,,7
.L152:
	movq (%ebp), %mm1
	pavgb (%edx,%ebp), %mm1
	movq %mm1, (%ecx)
	addl %edx,%ebp
	addl %edx,%ecx
	decl %esi
	jnz .L152

	jmp .L131
	.p2align 4,,7
.L148:
	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L159:
	movq (%ebp), %mm1
	pavgb (%edi,%ebp), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm1
	pavgb 8(%edi,%ebp), %mm1
	movq %mm1, 8(%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L159

	jmp .L131
	
.L220:
	.p2align 4,,7
	cmpl $0,20(%esp)
	jne .L162
	cmpl $0,92(%esp)
	je .L163

	cmpl $8,68(%esp)
	jne .L164

	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L168:
	movq (%ebp), %mm1
	pavgb 1(%ebp), %mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)
	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L168

	jmp .L131

	.p2align 4,,7
.L164:
	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L175:
	movq (%ebp), %mm1
	pavgb 1(%ebp), %mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm1
	pavgb 9(%ebp), %mm1
	pavgb 8(%ecx), %mm1
	movq %mm1, 8(%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L175

	jmp .L131

	.p2align 4,,7
.L163:
	cmpl $8,68(%esp)
	jne .L178

	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L182:
	movq (%ebp), %mm1
	pavgb 1(%ebp), %mm1
	movq %mm1, (%ecx)
	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L182

	jmp .L131
	.p2align 4,,7
.L178:
	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L189:
	movq (%ebp), %mm1
	pavgb 1(%ebp), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm1
	pavgb 9(%ebp), %mm1
	movq %mm1, 8(%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
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
	movl 64(%esp),%edi

	.p2align 4,,7
.L197:
	movq (%ebp), %mm0
	pavgb 1(%ebp), %mm0
	movq (%edi,%ebp), %mm1
	pavgb 1(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L197

	jmp .L131
	.p2align 4,,7
.L193:
	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L204:
	movq (%ebp), %mm0
	pavgb 1(%ebp), %mm0
	movq (%edi,%ebp), %mm1
	pavgb 1(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	pavgb (%ecx), %mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm0
	pavgb 9(%ebp), %mm0
	movq 8(%edi,%ebp), %mm1
	pavgb 9(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	pavgb 8(%ecx), %mm1
	movq %mm1, 8(%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L204

	jmp .L131
	.p2align 4,,7
.L192:
	cmpl $8,68(%esp)
	jne .L207

	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L211:
	movq (%ebp), %mm0
	pavgb 1(%ebp), %mm0
	movq (%edi,%ebp), %mm1
	pavgb 1(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	movq %mm1, (%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L211

	jmp .L131
	.p2align 4,,7
.L207:
	movl 72(%esp),%esi
	movl 64(%esp),%edi

	.p2align 4,,7
.L218:
	movq (%ebp), %mm0
	pavgb 1(%ebp), %mm0
	movq (%edi,%ebp), %mm1
	pavgb 1(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	movq %mm1, (%ecx)

	movq 8(%ebp), %mm0
	pavgb 9(%ebp), %mm0
	movq 8(%edi,%ebp), %mm1
	pavgb 9(%edi,%ebp), %mm1
	pavgb %mm0,%mm1
	movq %mm1, 8(%ecx)

	addl %edi,%ebp
	addl %edi,%ecx
	decl %esi
	jnz .L218
.L131:
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	addl $36,%esp
	ret
