#  
#
#   dist1_00.s:  SSE optimized version of the noninterpolative section of
#                dist1(), part of the motion vector detection code.  Runs
#                mpeg2enc -r11 at about 450% the speed of the original on
#                my Athlon.
#  
#   Copyright (C) 2000 Chris Atenasio <chris@crud.net>
#  

.global dist1_01_SSE

#  
#  int dist1_01(char *blk1,char *blk2,int lx,int h);

#  eax = p1
#  ebx = p2
#  ecx = counter temp
#  edx = lx;

#  mm0 = distance accumulator
#  mm1 = distlim
#  mm2 = rowsleft
#  mm3 = 2 (rows per loop)
#  mm4 = temp
#  mm5 = temp
#  mm6 = temp
#  

.align 32
dist1_01_SSE: 
        pushl %ebp
        movl %esp,%ebp

        pushl %ebx
        pushl %ecx
        pushl %edx

        pxor %mm0,%mm0          	#  zero acculumator 

        movl 8(%ebp),%eax       	#  get p1 
        movl 12(%ebp),%ebx      	#  get p2 
        movl 16(%ebp),%edx      	#  get lx 

        movl 20(%ebp),%ecx      	#  get rowsleft 
        jmp nextrow01           	#  snap to it 
.align 32
nextrow01: 
        movq (%eax),%mm4              	#  load first 8 bytes of p1 (row 1) 
        pavgb 1(%eax), %mm4
        psadbw (%ebx), %mm4
        paddd %mm4,%mm0                 #  accumulate difference 

        movq 8(%eax),%mm5              	#  load next 8 bytes of p1 (row 1) 
        pavgb 9(%eax),%mm5
        psadbw 8(%ebx),%mm5
        paddd %mm5,%mm0              	#  accumulate difference 

        addl %edx,%eax               	#  update pointer to next row 
        addl %edx,%ebx                	#  ditto 

        movq (%eax),%mm6               	#  load first 8 bytes of p1 (row 2) 
        pavgb 1(%eax),%mm6
        psadbw (%ebx),%mm6
        paddd %mm6,%mm0         	#  accumulate difference 

        movq 8(%eax),%mm7       	#  load next 8 bytes of p1 (row 2) 
        pavgb 9(%eax),%mm7
        psadbw 8(%ebx),%mm7
        paddd %mm7,%mm0         	#  accumulate difference 

        addl %edx,%eax          	#  update pointer to next row 
        addl %edx,%ebx          	#  ditto 

        subl $2,%ecx                    #  check rowsleft 
        jnz nextrow01           	#  rinse and repeat 

        movd %mm0,%eax          	#  store return value 

        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               	#  restore stack pointer 

        emms                    	#  clear mmx registers 
        ret                     	#  we now return you to your regular programming 

