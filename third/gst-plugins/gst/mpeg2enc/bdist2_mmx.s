# 
# 
#   bdist2_mmx.s:  mmX optimized bidirectional squared distance sum
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
#  squared error between a (16*h) block and a bidirectional
#  prediction
# 
#  p2: address of top left pel of block
#  pf,hxf,hyf: address and half pel flags of forward ref. block
#  pb,hxb,hyb: address and half pel flags of backward ref. block
#  h: height of block
#  lx: distance (in bytes) of vertically adjacent pels in p2,pf,pb
#  mmX version
# 

# 
# int bdist2_mmx(
# unsigned char *pf, unsigned char *pb, unsigned char *p2,
# int lx, int hxf, int hyf, int hxb, int hyb, int h)
# 
#   unsigned char *pfa,*pfb,*pfc,*pba,*pbb,*pbc;
#   int s;
# 


.text
.global bdist2_MMX

.align 32
bdist2_MMX: 
        pushl %ebp                      
        movl %esp,%ebp          
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl %esi
        pushl %edi

        subl      $32,%esp

        movl      32(%ebp), %edx
        movl      24(%ebp), %eax
        movl      20(%ebp), %esi

        movl      8(%ebp), %ecx
        addl      %eax,%ecx
        movl      %ecx, 4(%esp)
        movl      %esi,%ecx
        imull     28(%ebp), %ecx
        movl      8(%ebp), %ebx
        addl      %ebx,%ecx
        movl      %ecx, 8(%esp)
        addl      %ecx,%eax
        movl      %eax, 12(%esp)
        movl      12(%ebp), %eax
        addl      %edx,%eax
        movl      %eax, 16(%esp)
        movl      %esi,%eax
        imull     36(%ebp), %eax
        movl      12(%ebp), %ecx
        addl      %ecx,%eax
        movl      %eax, 20(%esp)
        addl      %eax,%edx
        movl      %edx, 24(%esp)
        xorl      %esi,%esi     
        movl      %esi,%eax

        movl      40(%ebp), %edi
        testl     %edi,%edi 
        jle       bdist2exit

        pxor      %mm7,%mm7
        pxor      %mm6,%mm6
        pcmpeqw   %mm5,%mm5
        psubw     %mm5,%mm6
        psllw     $1,%mm6

bdist2top: 
        movl      8(%ebp), %eax
        movl      4(%esp), %ebx
        movl      8(%esp), %ecx
        movl      12(%esp), %edx
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
        movq      (%ecx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        movq      (%edx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        paddw     %mm6,%mm0
        paddw     %mm6,%mm1
        psrlw     $2,%mm0
        psrlw     $2,%mm1

        movl      12(%ebp), %eax
        movl      16(%esp), %ebx
        movl      20(%esp), %ecx
        movl      24(%esp), %edx
        movq      (%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        movq      (%ebx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3
        movq      (%ecx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3
        movq      (%edx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3

        paddw     %mm6,%mm2
        paddw     %mm6,%mm3
        psrlw     $2,%mm2
        psrlw     $2,%mm3

        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        psrlw     $1,%mm6
        paddw     %mm6,%mm0
        paddw     %mm6,%mm1
        psllw     $1,%mm6
        psrlw     $1,%mm0
        psrlw     $1,%mm1

        movl      16(%ebp), %eax
        movq      (%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        psubw     %mm2,%mm0
        psubw     %mm3,%mm1
        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        paddd     %mm1,%mm0

        movd      %mm0,%eax
        psrlq     $32,%mm0
        movd      %mm0,%ebx
        addl      %eax,%esi
        addl      %ebx,%esi

        movl      8(%ebp), %eax
        movl      4(%esp), %ebx
        movl      8(%esp), %ecx
        movl      12(%esp), %edx
        movq      8(%eax),%mm0
        movq      %mm0,%mm1
        punpcklbw %mm7,%mm0
        punpckhbw %mm7,%mm1
        movq      8(%ebx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        movq      8(%ecx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        movq      8(%edx),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        paddw     %mm6,%mm0
        paddw     %mm6,%mm1
        psrlw     $2,%mm0
        psrlw     $2,%mm1

        movl      12(%ebp), %eax
        movl      16(%esp), %ebx
        movl      20(%esp), %ecx
        movl      24(%esp), %edx
        movq      8(%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3
        movq      8(%ebx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3
        movq      8(%ecx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3
        movq      8(%edx),%mm4
        movq      %mm4,%mm5
        punpcklbw %mm7,%mm4
        punpckhbw %mm7,%mm5
        paddw     %mm4,%mm2
        paddw     %mm5,%mm3
        paddw     %mm6,%mm2
        paddw     %mm6,%mm3
        psrlw     $2,%mm2
        psrlw     $2,%mm3

        paddw     %mm2,%mm0
        paddw     %mm3,%mm1
        psrlw     $1,%mm6
        paddw     %mm6,%mm0
        paddw     %mm6,%mm1
        psllw     $1,%mm6
        psrlw     $1,%mm0
        psrlw     $1,%mm1

        movl      16(%ebp), %eax
        movq      8(%eax),%mm2
        movq      %mm2,%mm3
        punpcklbw %mm7,%mm2
        punpckhbw %mm7,%mm3

        psubw     %mm2,%mm0
        psubw     %mm3,%mm1
        pmaddwd   %mm0,%mm0
        pmaddwd   %mm1,%mm1
        paddd     %mm1,%mm0

        movd      %mm0,%eax
        psrlq     $32,%mm0
        movd      %mm0,%ebx
        addl      %eax,%esi
        addl      %ebx,%esi

    	movl      20(%ebp), %eax
        addl      %eax, 16(%ebp)
        addl      %eax, 8(%ebp)
        addl      %eax, 4(%esp)
        addl      %eax, 8(%esp)
        addl      %eax, 12(%esp)
        addl      %eax, 12(%ebp)
        addl      %eax, 16(%esp)
        addl      %eax, 20(%esp)
        addl      %eax, 24(%esp)

        decl      %edi
        jg        bdist2top
    	movl      %esi,%eax

bdist2exit: 

        addl $32,%esp

        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               

        emms                    
        ret


