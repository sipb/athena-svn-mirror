	.align 16
.globl add_pred_mmx
	.type	 add_pred_mmx,@function
add_pred_mmx:
	pushl %edi
	pushl %esi
	movl 16(%esp),%esi
	movl 20(%esp),%ecx
	movl 24(%esp),%edi
	movl 28(%esp),%eax
	pxor %mm4, %mm4
	movl $8,%edx

	.p2align 4,,7
.L224:
	movq (%eax), %mm0
	movq 8(%eax), %mm1
	movq (%esi), %mm2
	movq %mm2, %mm6
	punpcklbw %mm4, %mm2
	punpckhbw %mm4, %mm6
	paddw %mm2, %mm0
	paddw %mm6, %mm1
	packuswb %mm1, %mm0
	movq %mm0, (%ecx)

	addl $16,%eax
	addl %edi,%esi
	addl %edi,%ecx

	decl %edx
	jg .L224

	emms
	popl %esi
	popl %edi
	ret

	.align 16
.globl sub_pred_mmx
	.type	 sub_pred_mmx,@function
sub_pred_mmx:
	pushl %edi
	pushl %esi
	movl 12(%esp),%esi
	movl 16(%esp),%ecx
	movl 20(%esp),%edi
	movl 24(%esp),%eax
	pxor %mm4, %mm4
	movl $8,%edx
	.p2align 4,,7
.L230:
	movq (%ecx), %mm0
	movq (%esi), %mm2
	movq %mm0, %mm1
	punpcklbw %mm4, %mm0
	punpckhbw %mm4, %mm1
	movq %mm2, %mm3
	punpcklbw %mm4, %mm2
	punpckhbw %mm4, %mm3
	psubw %mm2, %mm0
	psubw %mm3, %mm1
	movq %mm0, (%eax)
	movq %mm1, 8(%eax)

	addl $16,%eax
	addl %edi,%ecx
	addl %edi,%esi

	decl %edx
	jg .L230

	emms
	popl %esi
	popl %edi
	ret
