#  
 #  
#   dist2_mmx.s:  mmX optimized squared distance sum
 #  
#   Original believed to be Copyright (C) 2000 Brent Byeler
 #  
#   This program is free software; you can reaxstribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 2
#   of the License, or (at your option) any later version.
 #  
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
 #  
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 #  
#  

#  
#  total squared difference between two (16*h) blocks
#  including optional half pel interpolation of [ebp+8] ; blk1 (hx,hy)
#  blk1,blk2: addresses of top left pels of both blocks
#  lx:        distance (in bytes) of vertically adjacent pels
#  hx,hy:     flags for horizontal and/or vertical interpolation
#  h:         height of block (usually 8 or 16)
#  mmX version
#  

.global dist2_MMX
#  
#  int dist2_mmx(unsigned char *blk1, unsigned char *blk2,
#                  int lx, int hx, int hy, int h)

#  mm7 = 0

#  eax = pblk1 
#  ebx = pblk2
#  ecx = temp
#  edx = distance_sum
#  edi = h
#  esi = lx

#  
#   private constants needed
#  
#  

.data
.align 16
twos:   
 .word 2
.word 2
.word 2
.word 2

.align 32
dist2_MMX: 
        pushl %ebp                      	#  save frame pointer 
        movl %esp,%ebp          		#  link 
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl %esi
        pushl %edi

        movl    16(%ebp),%esi 			#  lx 
        movl    20(%ebp),%eax 			#  hx 
        movl    24(%ebp),%edx 			#  hy 
        movl    28(%ebp),%edi 			#  h  

    	pxor      %mm5,%mm5     		#  sum 
        testl     %edi,%edi    			#  h = 0? 
        jle       d2exit

        pxor      %mm7,%mm7    			#  get zeros i mm7 

        testl     %eax,%eax    			#  hx != 0? 
        jne       d2is10
        testl     %edx,%edx    			#  hy != 0? 
        jne       d2is10

        movl      8(%ebp),%eax
    	movl      12(%ebp),%ebx
        jmp      d2top00

.align 32
d2top00: 
        movq      (%eax),%mm0
        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1

        movq      (%ebx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        psubw     %mm2,%mm0
        psubw     %mm3,%mm1
        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        paddd     %mm1,%mm0

        movq      8(%eax),%mm1
        movq      %mm1,%mm2
        punpcklbw %mm7,%mm1
        punpckhbw %mm7,%mm2

        movq      8(%ebx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        psubw     %mm3,%mm1
        psubw     %mm4,%mm2
        pmaddwd   %mm1,%mm1
        pmaddwd   %mm2,%mm2
        paddd     %mm2,%mm1

        paddd     %mm1,%mm0
        paddd    %mm0,%mm5

        addl      %esi,%eax
        addl      %esi,%ebx
        decl      %edi
        jg        d2top00
        jmp       d2exit


d2is10: 
        testl     %eax,%eax
        je        d2is01
        testl     %edx,%edx
        jne       d2is01


        movl      8(%ebp),%eax 			#  blk1 
        movl      12(%ebp),%ebx 		#  blk1 

        pxor      %mm6,%mm6   			#  mm6 = 0 and isn't changed anyplace in the loop.. 
        pcmpeqw   %mm1,%mm1
        psubw     %mm1,%mm6
        jmp       d2top10

.align 32
d2top10: 
        movq      (%eax),%mm0
        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1
        movq      1(%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        paddw     %mm6,%mm0  			#  here we add mm6 = 0.... weird... 
        paddw     %mm6,%mm1
        psrlw     $1,%mm0
        psrlw     $1,%mm1

        movq      (%ebx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        psubw     %mm2,%mm0
        psubw     %mm3,%mm1
        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        paddd     %mm1,%mm0

        movq      8(%eax),%mm1
        movq      %mm1,%mm2
        punpcklbw %mm7,%mm1
        punpckhbw %mm7,%mm2
        movq      9(%eax),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4
        paddw     %mm3,%mm1
        paddw     %mm4,%mm2
        paddw     %mm6,%mm1
        paddw     %mm6,%mm2
        psrlw     $1,%mm1
        psrlw     $1,%mm2

        movq      8(%ebx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        psubw     %mm3,%mm1
        psubw     %mm4,%mm2
        pmaddwd   %mm1,%mm1
        pmaddwd   %mm2,%mm2
        paddd     %mm2,%mm1


        paddd     %mm1,%mm0
        paddd     %mm0,%mm5
        addl      %esi,%eax
        addl      %esi,%ebx
        decl      %edi
        jg        d2top10


        jmp       d2exit

d2is01: 
        testl     %eax,%eax
        jne       d2is11
        testl     %edx,%edx
        je        d2is11

        movl      8(%ebp),%eax 			#  blk1 
        movl      12(%ebp),%edx 		#  blk2 
        movl      %eax,%ebx
        addl      %esi,%ebx 			#   blk1 + lx 

        pxor      %mm6,%mm6
        pcmpeqw   %mm1,%mm1
        psubw     %mm1,%mm6 			#  mm6 = 1 
        jmp       d2top01

.align 32
d2top01: 
        movq      (%eax),%mm0
        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1
        movq      (%ebx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        paddw     %mm6,%mm0
        paddw     %mm6,%mm1
        psrlw     $1,%mm0
        psrlw     $1,%mm1

        movq      (%edx),%mm2
        movq      %mm2,%mm3
    	punpcklbw %mm7,%mm2
    	punpckhbw %mm7,%mm3

    	psubw     %mm2,%mm0
    	psubw     %mm3,%mm1

        pmaddwd   %mm0,%mm0
    	pmaddwd   %mm1,%mm1
    	paddd     %mm1,%mm0

        movq      8(%eax),%mm1
        movq      %mm1,%mm2
        punpcklbw %mm7,%mm1
        punpckhbw %mm7,%mm2

        movq      8(%ebx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        paddw     %mm3,%mm1
        paddw     %mm4,%mm2
        paddw     %mm6,%mm1
        paddw     %mm6,%mm2
        psrlw     $1,%mm1
        psrlw     $1,%mm2

        movq      8(%edx),%mm3
        movq      %mm3,%mm4
    	punpcklbw %mm7,%mm3
    	punpckhbw %mm7,%mm4

    	psubw     %mm3,%mm1
    	psubw     %mm4,%mm2

        pmaddwd   %mm1,%mm1
    	pmaddwd   %mm2,%mm2
    	paddd     %mm1,%mm0
        paddd     %mm2,%mm0

        #  Accumulate in "s" - we use mm5 for the purpose 
        #  
        #  movd     ecx, mm0
    	#  add       s, ecx
        #  psrlq    mm0, 32
        #  movd     ecx, mm0
        #  add              s, ecx
	#  
        paddd     %mm0,%mm5

        #  Originally this moved 
        movl      %ebx,%eax   			#  eax = eax + lx 
        addl      %esi,%edx   			#  edx = edx + lx 
        addl      %esi,%ebx   			#  ebx = ebx + lx 
        decl      %edi
        jg        d2top01
        jmp       d2exit

d2is11: 
        movl      8(%ebp),%eax 			#  blk1 
        movl      12(%ebp),%edx 		#  blk2 
        movl      %eax,%ebx 			#   blk1 
        addl      %esi,%ebx 			#  ebx = blk1 + lx 
        jmp       d2top11

.align 32
d2top11: 
        movq      (%eax),%mm0
        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1
        movq      1(%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        movq      (%ebx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        movq      1(%ebx),%mm4
        movq      %mm4,%mm6
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm6
        paddw     %mm4,%mm2
        paddw     %mm6,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
	#  
	#  pxor         mm6, mm6    ; mm6 = 0
        #  pcmpeqw          mm5, mm5    ; mm5 = -1
        #  psubw        mm6, mm5    ; mm6 = 1
        #  paddw        mm6, mm6    ; mm6 = 2
	#  
        movq      twos,%mm6
        paddw     %mm6,%mm0   			#  round mm0 
        paddw     %mm6,%mm1   			#  round mm1 
        psrlw     $2,%mm0
        psrlw     $2,%mm1

        movq      (%edx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        psubw     %mm2,%mm0
        psubw     %mm3,%mm1
        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        paddd     %mm1,%mm0

        movq      8(%eax),%mm1
        movq      %mm1,%mm2
        punpcklbw %mm7,%mm1
        punpckhbw %mm7,%mm2

        movq      9(%eax),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        paddw     %mm3,%mm1
        paddw     %mm4,%mm2

        movq      8(%ebx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4
        paddw     %mm3,%mm1
        paddw     %mm4,%mm2

        movq      9(%ebx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        paddw     %mm3,%mm1
        paddw     %mm4,%mm2

       	#  
        #  pxor     mm6, mm6    ; Zero mm6
        #  pcmpeqw          mm5, mm5    ; mm5 = -1
        #  psubw    mm6, mm5    ; mm6 = 1
        #  paddw    mm6, mm6    ; mm6 = 2
        #  paddw    mm1, mm6    ; round mm1 and mm2
        #  paddw    mm2, mm6
	#  
        movq      twos,%mm6
        paddw     %mm6,%mm1
        paddw     %mm6,%mm2

        psrlw     $2,%mm1
        psrlw     $2,%mm2

        movq      8(%edx),%mm3
        movq      %mm3,%mm4
        punpcklbw %mm7,%mm3
        punpckhbw %mm7,%mm4

        psubw     %mm3,%mm1
        psubw     %mm4,%mm2
        pmaddwd   %mm1,%mm1
        pmaddwd   %mm2,%mm2
        paddd     %mm2,%mm1

        paddd     %mm1,%mm0

        #  Accumulate the result in "s" we use mm6 for the purpose... 

 	#  
        #  movd     ecx, mm0
    	#  add       s, ecx
        #  psrlq    mm0, 32
        #  movd     ecx, mm0
        #  add      s, ecx
	#  
        paddd     %mm0,%mm5

        movl      %ebx,%eax   			#  ahem ebx = eax at start of loop and wasn't changed... 
        addl      %esi,%ebx
        addl      %esi,%edx
        decl      %edi
        jg        d2top11


d2exit: 
        #  Put the final sum in eax for return... 
        movd      %mm5,%eax
        psrlq     $32,%mm5
        movd      %mm5,%ecx
    	addl      %ecx,%eax

        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               		#  restore stack pointer 
	

        emms                    		#  clear mmx registers 
        ret

