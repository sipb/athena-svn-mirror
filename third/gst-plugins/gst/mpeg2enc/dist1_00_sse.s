#  
#  
#    dist1_00.s:  SSE optimized version of the noninterpolative section of
#                 dist1(), part of the motion vector detection code.  Runs
#                 mpeg2enc -r11 at about 450% the speed of the original on
#                 my Athlon.
#  
#    Copyright (C) 2000 Chris Atenasio <chris@crud.net>
#  

.global dist1_00_SSE

#  
#  int dist1_00(char *blk1,char *blk2,int lx,int h,int distlim);

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
dist1_00_SSE: 
        pushl %ebp                                      
        movl %esp,%ebp                          

        pushl %ebx
        pushl %ecx
        pushl %edx

        pxor %mm0,%mm0          #  zero acculumator 

        movl 8(%ebp),%eax       #  get p1 
        movl 12(%ebp),%ebx      #  get p2 
        movl 16(%ebp),%edx      #  get lx 

        movd 20(%ebp),%mm2      #  get rowsleft 
        movd 24(%ebp),%mm1      #  get distlim 
        movl $2,%ecx            #  1 loop does 2 rows 
        movd %ecx,%mm3          #  MMX(TM) it! 
        jmp nextrow00           #  snap to it 
.align 32
nextrow00: 
        movq (%eax),%mm4        #  load first 8 bytes of p1 (row 1) 
        psadbw (%ebx), %mm4
        paddd %mm4,%mm0         #  accumulate difference 

        movq 8(%eax),%mm5       #  load next 8 bytes of p1 (row 1) 
        psadbw 8(%ebx),%mm5
        paddd %mm5,%mm0         #  accumulate difference 

        addl %edx,%eax          #  update pointer to next row 
        addl %edx,%ebx          #  ditto 

        movq (%eax),%mm6        #  load first 8 bytes of p1 (row 2) 
        psadbw (%ebx),%mm6
        paddd %mm6,%mm0         #  accumulate difference 

        movq 8(%eax),%mm4       #  load next 8 bytes of p1 (row 2) 
        psadbw 8(%ebx),%mm4
        paddd %mm4,%mm0         #  accumulate difference 

        psubd %mm3,%mm2         #  decrease rowsleft 
        movq %mm1,%mm5          #  copy distlim 
        pcmpgtd %mm0,%mm5       #  distlim > dist? 
        pand %mm5,%mm2          #  mask rowsleft with answer 
        movd %mm2,%ecx          #  move rowsleft to ecx 

        addl %edx,%eax          #  update pointer to next row 
        addl %edx,%ebx          #  ditto 

        testl %ecx,%ecx         #  check rowsleft 
        jnz nextrow00           #  rinse and repeat 

        movd %mm0,%eax          #  store return value 

        popl %edx               #  pop pop 
        popl %ecx               #  fizz fizz 
        popl %ebx               #  ia86 needs a fizz instruction 

        popl %ebp               #  restore stack pointer 

        emms                    #  clear mmx registers 
        ret                     #  we now return you to your regular programming 

