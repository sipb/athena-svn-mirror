#  
 #  
#  variance of a (16*16) block, multiplied by 256
#  p:  address of top left pel of block
#  lx: distance (in bytes) of vertically adjacent pels
#  MMX version
#  

#  
# int variancemmx(
# unsigned char *p,
# int lx)
# {
#   unsigned int s2 = 0;
#

.text
.global variance_MMX

.align 32
variance_MMX: 
        pushl %ebp                      	#  save frame pointer
        movl %esp,%ebp          		#  link
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl %esi
        pushl %edi

        movl      8(%ebp), %eax
        movl      12(%ebp), %edi
        xorl      %ebx,%ebx              	#  s2 
        movl      %ebx,%ecx              	#  s 
        movl      $16,%esi               	#  loop 16 

        pxor      %mm7,%mm7              	#  get zeros in MM7 

vartop: 
        movq      (%eax),%mm0
        movq      8(%eax),%mm2

        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1

        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        movq      %mm0,%mm5
        paddusw   %mm1,%mm5
        paddusw   %mm2,%mm5
        paddusw   %mm3,%mm5

        movq      %mm5,%mm6
        punpcklwd %mm7,%mm5
        punpckhwd %mm7,%mm6
        paddd     %mm6,%mm5 			#  MM5 = two 32 bit sums 

        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        pmaddwd   %mm2,%mm2
        pmaddwd   %mm3,%mm3

        paddd     %mm1,%mm0
        paddd     %mm2,%mm0
        paddd     %mm3,%mm0

        movd      %mm5,%edx
        addl      %edx,%ecx
        psrlq     $32,%mm5
        movd      %mm5,%edx
        addl      %edx,%ecx
        movd      %mm0,%edx
        addl      %edx,%ebx
        psrlq     $32,%mm0
        movd      %mm0,%edx
        addl      %edx,%ebx

        addl      %edi,%eax
        decl      %esi
        jg        vartop

        imull     %ecx,%ecx
        shrl      $8,%ecx
        subl      %ecx,%ebx
        movl      %ebx,%eax

        #  Retore  
        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               		#  restore stack pointer 

        emms
        ret

