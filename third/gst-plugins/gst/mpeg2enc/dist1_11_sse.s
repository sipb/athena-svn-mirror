#  
 #  
#   dist1_10.s:  SSE optimized version of distance comparision
#                                half-pixel interpolation in x and y axes
#   Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
#   From code Copyright (C) 2000 Chris Atenasio <chris@crud.net>
#  


.global dist1_11_SSE

#  
#  int dist1_11(char *blk1,char *blk2,int lx,int h);

#  eax = p1
#  ebx = p2
#  ecx = counter temp
#  edx = lx;
#  edi = p1+lx  


#  mm0 = distance accumulator
#  mm2 = rowsleft
#  mm3 = 2 (rows per loop)
#  mm4 = temp
#  mm5 = temp
#  mm6 = temp
#  


.align 32
dist1_11_SSE: 
        pushl %ebp              	#  save stack pointer 
        movl %esp,%ebp          	#  so that we can do this 

        pushl %ebx             	 	#  save the pigs 
        pushl %ecx              	#  make them squeal 
        pushl %edx              	#  lets have pigs for every meal 
        pushl %edi

        pxor %mm0,%mm0          	#  zero acculumator 

        movl 8(%ebp),%eax       	#  get p1 
        movl 12(%ebp),%ebx      	#  get p2 
        movl 16(%ebp),%edx      	#  get lx 
        movl %eax,%edi
        addl %edx,%edi
        movl 20(%ebp),%ecx      	#  get rowsleft 
        jmp nextrow11           	#  snap to it 
.align 32
nextrow11: 
        movq (%eax),%mm4                #  load first 8 bytes of p1 (row 1) 
        pavgb (%edi),%mm4
        movq 1(%eax),%mm5
        pavgb 1(%edi),%mm5
        pavgb %mm5, %mm4
        psadbw (%ebx),%mm4
        paddd %mm4,%mm0                 #  accumulate difference 

        movq 8(%eax),%mm6               #  load next 8 bytes of p1 (row 1) 
        pavgb 8(%edi),%mm6
        movq 9(%eax),%mm7
        pavgb 9(%edi),%mm7
        pavgb %mm7, %mm6
        psadbw 8(%ebx),%mm6
        paddd %mm6,%mm0                 #  accumulate difference 

        addl %edx,%eax                  #  update pointer to next row 
        addl %edx,%ebx                  #  ditto 
        addl %edx,%edi

        movq (%eax),%mm4                #  load first 8 bytes of p1 (row 1) 
        pavgb (%edi),%mm4
        movq 1(%eax),%mm5
        pavgb 1(%edi),%mm5
        pavgb %mm5, %mm4
        psadbw (%ebx),%mm4
        paddd %mm4,%mm0                 #  accumulate difference 

        movq 8(%eax),%mm6               #  load next 8 bytes of p1 (row 1) 
        pavgb 8(%edi),%mm6
        movq 9(%eax),%mm7
        pavgb 9(%edi),%mm7
        pavgb %mm7, %mm6
        psadbw 8(%ebx),%mm6
        paddd %mm6,%mm0                 #  accumulate difference 

        addl %edx,%eax          	#  update pointer to next row 
        addl %edx,%ebx          	#  ditto 
        addl %edx,%edi


        subl $2,%ecx                    #  check rowsleft 
        jnz nextrow11                   #  rinse and repeat 

        movd %mm0,%eax                  #  store return value 

        popl %edi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               	#  restore stack pointer 

        emms                    	#  clear mmx registers 
        ret                     	#  we now return you to your regular programming 

