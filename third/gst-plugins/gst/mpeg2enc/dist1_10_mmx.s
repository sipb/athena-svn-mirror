#  
#  
#   dist1_01_mmx.s:  MMX1 optimised 7bit*8 word absolute difference sum
#      We're reduce to seven bits as otherwise we also have to mess
#      horribly with carries and signed only comparisons make the code
#      simply enormous (and probably barely faster than a simple loop).
#      Since signals with a bona-fide 8bit res will be rare we simply
#      take the precision hit...
#      Actually we don't worry about carries from the low-order bits
#      either so 1/4 of the time we'll be 1 too low...
#  
#   Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
#  


.global dist1_10_MMX

#  
#  int dist1_10_MMX(unsigned char *p1,unsigned char *p2,int lx,int h);

#  esi = p1 (init:               blk1)
#  edi = p2 (init:               blk2)
#  ebx = p1+lx
#  ecx = rowsleft (init:  h)
#  edx = lx;

#  mm0 = distance accumulators (4 words)
#  mm1 = bytes p2
#  mm2 = bytes p1
#  mm3 = bytes p1+1
#  mm4 = temp 4 bytes in words interpolating p1, p1+1
#  mm5 = temp 4 bytes in words from p2
#  mm6 = temp comparison bit mask p1,p2
#  mm7 = temp comparison bit mask p2,p1
#  

.align 32
dist1_10_MMX: 
        pushl %ebp              	#  save stack pointer 
        movl %esp,%ebp  		#  so that we can do this 

        pushl %ebx              	#  Saves registers (called saves convention in 
        pushl %ecx              	#  x86 GCC it seems) 
        pushl %edx              	#   
        pushl %esi
        pushl %edi

        pxor %mm0,%mm0              	#  zero acculumators 

        movl 8(%ebp),%esi           	#  get p1 
        movl 12(%ebp),%edi             	#  get p2 
        movl 16(%ebp),%edx            	#  get lx 
        movl 20(%ebp),%ecx            	#  rowsleft := h 
        movl %esi,%ebx
    addl %edx,%ebx
        jmp nextrowmm10              	#  snap to it 
.align 32
nextrowmm10: 

                #  
                #  First 8 bytes of row
                #  

                #  First 4 bytes of 8
		#  

        movq (%esi),%mm4          	#  mm4 := first 4 bytes p1 
        pxor %mm7,%mm7
        movq %mm4,%mm2               	#   mm2 records all 8 bytes 
        punpcklbw %mm7,%mm4           	#   First 4 bytes p1 in Words... 

        movq (%ebx),%mm6             	#   mm6 := first 4 bytes p1+lx 
        movq %mm6,%mm3              	#   mm3 records all 8 bytes 
        punpcklbw %mm7,%mm6
        paddw %mm6,%mm4             	#   mm4 := First 4 bytes interpolated in words 
        psrlw $1,%mm4

        movq (%edi),%mm5              	#  mm5:=first 4 bytes of p2 in words 
        movq %mm5,%mm1
        punpcklbw %mm7,%mm5

        movq  %mm4,%mm7
        pcmpgtw %mm5,%mm7       	#  mm7 := [i : W0..3,mm4>mm5] 

        movq  %mm4,%mm6         	#  mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] 
        psubw %mm5,%mm6
        pand  %mm7,%mm6

        paddw %mm6,%mm0              	#  Add to accumulator 

        movq  %mm5,%mm6     		#  mm6 := [i : W0..3,mm5>mm4] 
        pcmpgtw %mm4,%mm6
        psubw %mm4,%mm5         	#  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] 
        pand  %mm6,%mm5

        paddw %mm5,%mm0               	#  Add to accumulator 

                #  Second 4 bytes of 8 

        movq %mm2,%mm4              	#  mm4 := Second 4 bytes p1 in words 
        pxor  %mm7,%mm7
        punpckhbw %mm7,%mm4
        movq %mm3,%mm6                  #  mm6 := Second 4 bytes p1+1 in words   
        punpckhbw %mm7,%mm6

        paddw %mm6,%mm4         	#   mm4 := First 4 Interpolated bytes in words 
        psrlw $1,%mm4

        movq %mm1,%mm5                  #  mm5:= second 4 bytes of p2 in words 
        punpckhbw %mm7,%mm5

        movq  %mm4,%mm7
        pcmpgtw %mm5,%mm7       	#  mm7 := [i : W0..3,mm4>mm5] 

        movq  %mm4,%mm6         	#  mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] 
        psubw %mm5,%mm6
        pand  %mm7,%mm6

        paddw %mm6,%mm0              	#  Add to accumulator 

        movq  %mm5,%mm6     		#  mm6 := [i : W0..3,mm5>mm4] 
        pcmpgtw %mm4,%mm6
        psubw %mm4,%mm5         	#  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] 
        pand  %mm6,%mm5

        paddw %mm5,%mm0                	#  Add to accumulator 


                #  
                #  Second 8 bytes of row
                #  
                #  First 4 bytes of 8
		#  

        movq 8(%esi),%mm4             	#  mm4 := first 4 bytes p1+8 
        pxor %mm7,%mm7
        movq %mm4,%mm2                 	#   mm2 records all 8 bytes 
        punpcklbw %mm7,%mm4           	#   First 4 bytes p1 in Words... 

        movq 8(%ebx),%mm6            	#   mm6 := first 4 bytes p1+lx+8 
        movq %mm6,%mm3              	#   mm3 records all 8 bytes 
        punpcklbw %mm7,%mm6
        paddw %mm6,%mm4             	#   mm4 := First 4 bytes interpolated in words 
        psrlw $1,%mm4

        movq 8(%edi),%mm5           	#  mm5:=first 4 bytes of p2+8 in words 
        movq %mm5,%mm1
        punpcklbw %mm7,%mm5

        movq  %mm4,%mm7
        pcmpgtw %mm5,%mm7       	#  mm7 := [i : W0..3,mm4>mm5] 

        movq  %mm4,%mm6         	#  mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] 
        psubw %mm5,%mm6
        pand  %mm7,%mm6

        paddw %mm6,%mm0              	#  Add to accumulator 

        movq  %mm5,%mm6     		#  mm6 := [i : W0..3,mm5>mm4] 
        pcmpgtw %mm4,%mm6
        psubw %mm4,%mm5         	#  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] 
        pand  %mm6,%mm5

        paddw %mm5,%mm0              	#  Add to accumulator 

                #  Second 4 bytes of 8 

        movq %mm2,%mm4              	#  mm4 := Second 4 bytes p1 in words 
        pxor  %mm7,%mm7
        punpckhbw %mm7,%mm4
        movq %mm3,%mm6                  #  mm6 := Second 4 bytes p1+1 in words   
        punpckhbw %mm7,%mm6

        paddw %mm6,%mm4         	#   mm4 := First 4 Interpolated bytes in words 
        psrlw $1,%mm4

        movq %mm1,%mm5                  #  mm5:= second 4 bytes of p2 in words 
        punpckhbw %mm7,%mm5

        movq  %mm4,%mm7
        pcmpgtw %mm5,%mm7       	#  mm7 := [i : W0..3,mm4>mm5] 

        movq  %mm4,%mm6         	#  mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] 
        psubw %mm5,%mm6
        pand  %mm7,%mm6

        paddw %mm6,%mm0            	#  Add to accumulator 

        movq  %mm5,%mm6     		#  mm6 := [i : W0..3,mm5>mm4] 
        pcmpgtw %mm4,%mm6
        psubw %mm4,%mm5         	#  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] 
        pand  %mm6,%mm5

        paddw %mm5,%mm0              	#  Add to accumulator 


                #  
                #       Loop termination condition... and stepping
                #               
		#  

        addl %edx,%esi          	#  update pointer to next row 
        addl %edx,%edi          	#  ditto 
        addl %edx,%ebx

        subl $1,%ecx
        testl %ecx,%ecx         	#  check rowsleft 
        jnz nextrowmm10

                #  Sum the Accumulators 
        movq  %mm0,%mm4
        psrlq $32,%mm4
        paddw %mm4,%mm0
        movq  %mm0,%mm6
        psrlq $16,%mm6
        paddw %mm6,%mm0
        movd %mm0,%eax          	#  store return value 
        andl $0xffff,%eax


        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               	#  restore stack pointer 

        emms                    	#  clear mmx registers 
        ret                     	#  we now return you to your regular programming 

