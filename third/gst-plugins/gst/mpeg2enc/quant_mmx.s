#
#  quantize_ni_mmx.s:  MMX optimized coefficient quantization sub-routine
#
#  Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
#
#  This program is free software; you can reaxstribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
#
#


.global quantize_ni_mmx
# int quantize_ni_mmx(short *dst, short *src, 
#                             short *quant_mat, short *i_quant_mat,
#                     int imquant, int mquant, int sat_limit)

#  See quantize.c: quant_non_intra_inv()  for reference implementation in C...
                ##  mquant is not currently used.
# eax = row counter...
# ebx = pqm
# ecx = piqm  ; Matrix of quads first (2^16/quant) 
                          # then (2^16/quant)*(2^16%quant) the second part is for rounding
# edx = satlim
# edi = psrc
# esi = pdst

# mm0 = [imquant|0..3]W
# mm1 = [sat_limit|0..3]W
# mm2 = *psrc -> src
# mm3 = rounding corrections...
# mm4 = flags...
# mm5 = saturation accumulator...
# mm6 = nzflag accumulator...
# mm7 = temp

                ## 
                ##  private constants needed
                ## 

.data
.align 16
.type satlim,@object
satlim: 
  .short 1024-1, 1024-1, 1024-1, 1024-1
.bss
.align 32
.type quant_buf,@object
quant_buf: 
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0
  .short 0,0,0,0,0,0,0,0

.text

.align 32
quantize_ni_mmx: 
        pushl %ebp                              # save frame pointer
        movl %esp,%ebp          # link
        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl %esi
        pushl %edi

        movl $quant_buf, %edi # get temporary dst w
        movl 12(%ebp),%esi      # get psrc
        movl 16(%ebp),%ebx      # get pqm
        movl 20(%ebp),%ecx  # get piqm
        movd 24(%ebp),%mm0  # get imquant (2^16 / mquant )
        movq %mm0,%mm1
        punpcklwd %mm1,%mm0
        punpcklwd %mm0,%mm0   # mm0 = [imquant|0..3]W

        pxor  %mm6,%mm6                 # saturation / out-of-range accumulator(s)

        movd 32(%ebp),%mm1  # sat_limit
        movq %mm1,%mm2
        punpcklwd %mm2,%mm1  # [sat_limit|0..3]W
        punpcklwd %mm1,%mm1  # mm1 = [sat_limit|0..3]W

        xorl      %edx,%edx # Non-zero flag accumulator 
        movl $16,%eax           # 16 quads to do 
        jmp nextquadniq

.align 32
nextquadniq: 
        movq (%esi),%mm2                        # mm0 = *psrc

        pxor    %mm4,%mm4
        pcmpgtw %mm2,%mm4      # mm4 = *psrc < 0
        pxor    %mm7,%mm7
        psubw   %mm2,%mm7      # mm7 = -*psrc
        psllw   $1,%mm7        # mm7 = -2*(*psrc)
        pand    %mm4,%mm7      # mm7 = -2*(*psrc)*(*psrc < 0)
        paddw   %mm7,%mm2      # mm2 = abs(*psrc)

        ##
        ##  Check whether we'll saturate intermediate results
        ##

        movq    %mm2,%mm7
        pcmpgtw satlim,%mm7      # Tooo  big for 16 bit arithmetic :-( (should be *very* rare)
        por     %mm7,%mm6

        ##
        ## Carry on with the arithmetic...
        psllw   $5,%mm2        # mm2 = 32*abs(*psrc)
        movq    (%ebx),%mm7    # mm7 = *pqm>>1
        psrlw   $1,%mm7
        paddw   %mm7,%mm2      # mm2 = 32*abs(*psrc)+((*pqm)) = "p"


        ##
        ## Do the first multiplication.  Cunningly we've set things up so
        ## it is exactly the top 16 bits we're interested in...
        ##
        ## We need the low word results for a rounding correction.  
        ## This is *not* exact (that actual
    ## correction the product abs(*psrc)*(*pqm)*(2^16%*qm) >> 16
    ##  However we get very very few wrong and none too low (the most
    ## important) and no errors for small coefficients (also important)
        ##      if we simply add abs(*psrc)

        movq    %mm2,%mm3
        pmullw  (%ecx),%mm3
        movq    %mm2,%mm5
        psrlw   $1,%mm5           # Want to see if adding p would carry into upper 16 bits
        psrlw   $1,%mm3
        paddw   %mm5,%mm3
        psrlw   $15,%mm3          # High bit in lsb rest 0's
        pmulhw  (%ecx),%mm2       # mm2 = (p*iqm+p) >> IQUANT_SCALE_POW2 ~= p/*qm



        ##
        ## To hide the latency lets update some pointers...
        addl  $8,%esi                                   # 4 word's
        addl  $8,%ecx                                   # 4 word's
        subl  $1,%eax

        ## Add rounding correction....
        paddw   %mm3,%mm2


        ##
        ## Do the second multiplication, again we ned to make a rounding adjustment
        movq    %mm2,%mm3
        pmullw  %mm0,%mm3
        movq    %mm2,%mm5
        psrlw   $1,%mm5           # Want to see if adding p would carry into upper 16 bits
        psrlw   $1,%mm3
        paddw   %mm5,%mm3
        psrlw   $15,%mm3          # High bit in lsb rest 0's

        pmulhw  %mm0,%mm2    # mm2 ~= (p/(qm*mquant)) 

        ##
        ## To hide the latency lets update some more pointers...
        addl  $8,%edi
        addl  $8,%ebx

        ## Correct rounding and the factor of two (we want p/(qm*2*mquant)
        paddw %mm3,%mm2
        psrlw $1,%mm2


        ##
        ## Check for saturation
        ##
        movq %mm2,%mm7
        pcmpgtw %mm1,%mm7
        por     %mm7,%mm6      # Accumulate that bad things happened...

        ##
        ##  Accumulate non-zero flags
        movq   %mm2,%mm7
        movq   %mm2,%mm5
        psrlq   $32,%mm5
        por     %mm7,%mm5
        movd    %edx,%mm7       ## edx  |= mm2 != 0
        por     %mm5,%mm7
        movd    %mm7,%edx


        ##
        ## Now correct the sign mm4 = *psrc < 0
        ##

        pxor %mm7,%mm7       # mm7 = -2*mm2
        psubw %mm2,%mm7
        psllw $1,%mm7
        pand  %mm4,%mm7      # mm7 = -2*mm2 * (*psrc < 0)
        paddw %mm7,%mm2      # mm7 = samesign(*psrc, mm2 )

                ##
                ##  Store the quantised words....
                ##

        movq %mm2,-8(%edi)
        testl %eax,%eax

        jnz nextquadniq

quit: 

        ## Return saturation in low word and nzflag in high word of reuslt dword 

        movq %mm6,%mm0
        psrlq $32,%mm0
        por   %mm0,%mm6
        movd  %mm6,%eax
        movl  %eax,%ebx
        shrl  $16,%ebx
        orl   %ebx,%eax
        andl  $0xffff,%eax              ## low word eax is saturation

        testl %eax,%eax
        jnz   skipupdate

        movl  $8,%ecx          ## 8 pairs of quads...
        movl  8(%ebp),%edi     ## destination
        movl  $quant_buf, %esi
update: 
        movq  (%esi),%mm0
        movq  8(%esi),%mm1
        addl  $16,%esi
        movq  %mm0,(%edi)
        movq  %mm1,8(%edi)
        addl  $16,%edi
        subl  $1,%ecx
        jnz   update
skipupdate: 
        movl  %edx,%ebx
        shll  $16,%ebx
        orl   %ebx,%edx
    andl  $0xffff0000,%edx ## hiwgh word ecx is nzflag
        orl   %edx,%eax


        popl %edi
        popl %esi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               # restore stack pointer

        emms                    # clear mmx registers
        ret


# int quant_weight_coeff_sum_mmx( short *blk, unsigned short * i_quant_mat )
#
# Simply add up the sum of coefficients weighted by their quantisation coefficients

.global quant_weight_coeff_sum_mmx
.align 32
quant_weight_coeff_sum_mmx: 
        pushl %ebp                              # save frame pointer
        movl %esp,%ebp          # link

        pushl %ecx
        pushl %esi
        pushl %edi

        movl 8(%ebp),%edi       # get pdst
        movl 12(%ebp),%esi      # get piqm

        movl $16,%ecx                   # 16 coefficient / quantiser quads to process...
        pxor %mm6,%mm6          # Accumulator
        pxor %mm7,%mm7          # Zero
quantsum: 
        movq    (%edi),%mm0
        movq    (%esi),%mm2

        ##
        ##      Compute absolute value of coefficients...
        ##
        movq    %mm7,%mm1
        pcmpgtw %mm0,%mm1  # (mm0 < 0 )
        movq    %mm0,%mm3
        psllw   $1,%mm3    # 2*mm0
        pand    %mm1,%mm3  # 2*mm0 * (mm0 < 0)
        psubw   %mm3,%mm0  # mm0 = abs(mm0)


        ##
        ## Compute the low and high words of the result....
        ## 
        movq    %mm0,%mm1
        pmullw  %mm2,%mm0
        addl            $8,%edi
        addl            $8,%esi
        pmulhw  %mm2,%mm1

        movq      %mm0,%mm3
        punpcklwd  %mm1,%mm3
        punpckhwd  %mm1,%mm0
        paddd      %mm3,%mm6
        paddd      %mm0,%mm6


        subl $1,%ecx
        jnz   quantsum

        movd   %mm6,%eax
        psrlq  $32,%mm6
        movd   %mm6,%ecx
        addl   %ecx,%eax

        popl %edi
        popl %esi
        popl %ecx

        popl %ebp               # restore stack pointer

        emms                    # clear mmx registers
        ret



