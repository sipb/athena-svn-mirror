# 
# 
#   fdist1_00.s:  MMX1 optimised 8*8 word absolute difference sum
# 
#   Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
#   Based on original GPL Copyright (C) 2000 Chris Atenasio <chris@crud.net>
# 

.global dist1_00_MMX

# 
#  int dist1_MMX(unsigned char *blk1,unsigned char *blk2,int lx,int h, int distlim);
#  N.b. distlim is *ignored* as testing for it is more expensive than the
#  occasional saving by aborting the computionation early...
#  esi = p1 (init:               blk1)
#  edi = p2 (init:               blk2)
#  ebx = distlim  
#  ecx = rowsleft (init:  h)
#  edx = lx;

#  mm0 = distance accumulators (4 words)
#  mm1 = temp 
#  mm2 = temp 
#  mm3 = temp
#  mm4 = temp
#  mm5 = temp 
#  mm6 = 0
#  mm7 = temp
# 


.align 32
dist1_00_MMX: 
        pushl %ebp              	#  save frame pointer 
        movl %esp,%ebp

        pushl %ebx              	#  Saves registers (called saves convention in 
        pushl %ecx              	#  x86 GCC it seems) 
        pushl %edx              	#   
        pushl %esi
        pushl %edi

        pxor %mm0,%mm0                 	#  zero acculumators 
        pxor %mm6,%mm6
        movl 8(%ebp),%esi               #  get p1 
        movl 12(%ebp),%edi              #  get p2 
        movl 16(%ebp),%edx              #  get lx 
        movl 20(%ebp),%ecx              #  get rowsleft 
        jmp nextrowmm00                 #  snap to it 
.align 32
nextrowmm00: 
        movq (%esi),%mm4        	#  load first 8 bytes of p1 row  
        movq (%edi),%mm5        	#  load first 8 bytes of p2 row 

        movq %mm4,%mm7      		#  mm5 = abs(mm4-mm5) 
        psubusb %mm5,%mm7
        psubusb %mm4,%mm5
        paddb   %mm7,%mm5

        #   Add the abs(mm4-mm5) bytes to the accumulators 
        movq  %mm5,%mm7                 #  mm7 := [i :   B0..3, mm1]W 
        punpcklbw %mm6,%mm7
        paddw %mm7,%mm0
        punpckhbw %mm6,%mm5
        paddw %mm5,%mm0


        movq 8(%esi),%mm4               #  load second 8 bytes of p1 row  
        movq 8(%edi),%mm5               #  load second 8 bytes of p2 row 

        movq %mm4,%mm7      		#  mm5 = abs(mm4-mm5) 
        psubusb %mm5,%mm7
        psubusb %mm4,%mm5
        paddb   %mm7,%mm5

        #   Add the abs(mm4-mm5) bytes to the accumulators 
        movq  %mm5,%mm7                 #  mm7 := [i :   B0..3, mm1]W 
        punpcklbw %mm6,%mm7
        punpckhbw %mm6,%mm5
        paddw %mm7,%mm0

        addl %edx,%esi          	#  update pointer to next row 
        addl %edx,%edi          	#  ditto  

        paddw %mm5,%mm0

        subl $1,%ecx
        jnz nextrowmm00

returnmm00:     

                #  Sum the Accumulators 
        movq  %mm0,%mm5                 #  mm5 := [W0+W2,W1+W3, mm0 
        psrlq $32,%mm5
        movq  %mm0,%mm4
        paddw %mm5,%mm4

        movq  %mm4,%mm7             	#  mm6 := [W0+W2+W1+W3, mm0] 
        psrlq $16,%mm7
        paddw %mm7,%mm4
        movd %mm4,%eax          	#  store return value 
        andl $0xffff,%eax

        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp

        emms                    	#  clear mmx registers 
        ret

