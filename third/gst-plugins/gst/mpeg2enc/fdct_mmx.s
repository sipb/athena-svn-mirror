# //////////////////////////////////////////////////////////////////////////////
# //
# //  fdctam32.c - AP922 MMX(3D-Now) forward-DCT
# //  ----------
# //  Intel Application Note AP-922 - fast, precise implementation of DCT
# //        http://developer.intel.com/vtune/cbts/appnotes.htm
# //  ----------
# //  
# //       This routine can use a 3D-Now/MMX enhancement to increase the
# //  accuracy of the fdct_col_4 macro.  The dct_col function uses 3D-Now's
# //  PMHULHRW instead of MMX's PMHULHW(and POR).  The substitution improves
# //  accuracy very slightly with performance penalty.  If the target CPU
# //  does not support 3D-Now, then this function cannot be executed.
# //  
# //  For a fast, precise MMX implementation of inverse-DCT 
# //              visit http://www.elecard.com/peter
# //
# //  v1.0 07/22/2000 (initial release)
# //     
# //  liaor@iname.com  http://members.tripod.com/~liaor  
# //////////////////////////////////////////////////////////////////////////////

###
### A.Stevens Jul 2000:  ported to nasm syntax and disentangled from
### from Win**** compiler specific stuff.
### All the real work was done above though.
### See above for how to optimise quality on 3DNow! CPU's


                ##
                ## Constants for DCT
                ##

.equ BITS_FRW_ACC,  3      # 2 or 3 for accuracy
.equ SHIFT_FRW_COL, BITS_FRW_ACC
.equ SHIFT_FRW_ROW, (BITS_FRW_ACC + 17)
.equ RND_FRW_ROW,   (1 << (SHIFT_FRW_ROW-1))
.equ RND_FRW_COL,   (1 << (SHIFT_FRW_COL-1))

.extern fdct_one_corr
.extern fdct_r_row                              #  Defined in C for convenience
                ##
                ## Concatenated table of forward dct transformation coeffs.
                ## 
.extern fdct_tg_all_16                  # Defined in C for convenience


                ##
                ## Concatenated table of forward dct coefficients
                ## 
.extern tab_frw_01234567        # Defined in C for convenience

                ## Offsets into table..
.text

.global fdct_mmx

### 
### void fdct_mmx( short *blk )
### 



#     ////////////////////////////////////////////////////////////////////////
#     //
#     // The high-level pseudocode for the fdct_am32() routine :
#     //
#     // fdct_am32()
#     // {
#     //    forward_dct_col03(); // dct_column transform on cols 0-3
#     //    forward_dct_col47(); // dct_column transform on cols 4-7
#     //    for ( j = 0; j < 8; j=j+1 )
#     //      forward_dct_row1(j); // dct_row transform on row #j
#     // }
#     //
#    

.align 32
fdct_mmx: 
        pushl %ebp                      # save stack pointer
        movl %esp,%ebp          # link

        pushl %ebx
        pushl %ecx
        pushl %edx
        pushl %edi

        mov 8(%ebp), %eax
    #// transform the left half of the matrix (4 columns)

    lea fdct_tg_all_16, %ebx
    mov %ecx, %eax

#       lea round_frw_col,  [r_frw_col]
    # for ( i = 0; i < 2; i = i + 1)
    # the for-loop is executed twice.  We are better off unrolling the 
    # loop to avoid branch misprediction.
mmx32_fdct_col03: 
    movq 16(%eax),%mm0   # 0 ; x1
     ##

    movq 96(%eax),%mm1   # 1 ; x6
    movq %mm0,%mm2 # 2 ; x1

    movq 32(%eax),%mm3   # 3 ; x2
    paddsw %mm1,%mm0 # t1 = x[1] + x[6]

    movq 80(%eax),%mm4   # 4 ; x5
    psllw $SHIFT_FRW_COL,%mm0 # t1

    movq (%eax),%mm5   # 5 ; x0
    paddsw %mm3,%mm4 # t2 = x[2] + x[5]

    paddsw 112(%eax),%mm5   # t0 = x[0] + x[7]
    psllw $SHIFT_FRW_COL,%mm4 # t2

    movq %mm0,%mm6 # 6 ; t1
    psubsw %mm1,%mm2 # 1 ; t6 = x[1] - x[6]

    movq 8(%ebx),%mm1    # 1 ; tg_2_16
    psubsw %mm4,%mm0 # tm12 = t1 - t2

    movq 48(%eax),%mm7   # 7 ; x3
    pmulhw %mm0,%mm1 # tm12*tg_2_16

    paddsw 64(%eax),%mm7   # t3 = x[3] + x[4]
    psllw $SHIFT_FRW_COL,%mm5 # t0

    paddsw %mm4,%mm6 # 4 ; tp12 = t1 + t2
    psllw $SHIFT_FRW_COL,%mm7 # t3

    movq %mm5,%mm4 # 4 ; t0
    psubsw %mm7,%mm5 # tm03 = t0 - t3

    paddsw %mm5,%mm1 # y2 = tm03 + tm12*tg_2_16
    paddsw %mm7,%mm4 # 7 ; tp03 = t0 + t3

    por fdct_one_corr,%mm1    # correction y2 +0.5
    psllw $SHIFT_FRW_COL+1,%mm2 # t6

    pmulhw 8(%ebx),%mm5    # tm03*tg_2_16
    movq %mm4,%mm7 # 7 ; tp03

    psubsw 80(%eax),%mm3   # t5 = x[2] - x[5]
    psubsw %mm6,%mm4 # y4 = tp03 - tp12

    movq %mm1,32(%ecx)   # 1 ; save y2
    paddsw %mm6,%mm7 # 6 ; y0 = tp03 + tp12

    movq 48(%eax),%mm1   # 1 ; x3
    psllw $SHIFT_FRW_COL+1,%mm3 # t5

    psubsw 64(%eax),%mm1   # t4 = x[3] - x[4]
    movq %mm2,%mm6 # 6 ; t6

    movq %mm4,64(%ecx)   # 4 ; save y4
    paddsw %mm3,%mm2 # t6 + t5

    pmulhw 32(%ebx),%mm2    # tp65 = (t6 + t5)*cos_4_16
    psubsw %mm3,%mm6 # 3 ; t6 - t5

    pmulhw 32(%ebx),%mm6    # tm65 = (t6 - t5)*cos_4_16
    psubsw %mm0,%mm5 # 0 ; y6 = tm03*tg_2_16 - tm12

    por fdct_one_corr,%mm5    # correction y6 +0.5
    psllw $SHIFT_FRW_COL,%mm1 # t4

    por fdct_one_corr,%mm2    # correction tp65 +0.5
    movq %mm1,%mm4 # 4 ; t4

    movq (%eax),%mm3   # 3 ; x0
    paddsw %mm6,%mm1 # tp465 = t4 + tm65

    psubsw 112(%eax),%mm3   # t7 = x[0] - x[7]
    psubsw %mm6,%mm4 # 6 ; tm465 = t4 - tm65

    movq (%ebx),%mm0    # 0 ; tg_1_16
    psllw $SHIFT_FRW_COL,%mm3 # t7

    movq 16(%ebx),%mm6    # 6 ; tg_3_16
    pmulhw %mm1,%mm0 # tp465*tg_1_16

    movq %mm7,(%ecx)   # 7 ; save y0
    pmulhw %mm4,%mm6 # tm465*tg_3_16

    movq %mm5,96(%ecx)   # 5 ; save y6
    movq %mm3,%mm7 # 7 ; t7

    movq 16(%ebx),%mm5    # 5 ; tg_3_16
    psubsw %mm2,%mm7 # tm765 = t7 - tp65

    paddsw %mm2,%mm3 # 2 ; tp765 = t7 + tp65
    pmulhw %mm7,%mm5 # tm765*tg_3_16

    paddsw %mm3,%mm0 # y1 = tp765 + tp465*tg_1_16
    paddsw %mm4,%mm6 # tm465*tg_3_16

    pmulhw (%ebx),%mm3    # tp765*tg_1_16
    ##

    por fdct_one_corr,%mm0    # correction y1 +0.5
    paddsw %mm7,%mm5 # tm765*tg_3_16

    psubsw %mm6,%mm7 # 6 ; y3 = tm765 - tm465*tg_3_16
    add $8,%eax

    movq %mm0,16(%ecx)   # 0 ; save y1
    paddsw %mm4,%mm5 # 4 ; y5 = tm765*tg_3_16 + tm465

    movq %mm7,48(%ecx)   # 7 ; save y3
    psubsw %mm1,%mm3 # 1 ; y7 = tp765*tg_1_16 - tp465

    movq %mm5,80(%ecx)   # 5 ; save y5


mmx32_fdct_col47:  # begin processing last four columns
    movq 16(%eax),%mm0   # 0 ; x1
    ##
    movq %mm3,112(%ecx)   # 3 ; save y7 (columns 0-4)
    ##

    movq 96(%eax),%mm1   # 1 ; x6
    movq %mm0,%mm2 # 2 ; x1

    movq 32(%eax),%mm3   # 3 ; x2
    paddsw %mm1,%mm0 # t1 = x[1] + x[6]

    movq 80(%eax),%mm4   # 4 ; x5
    psllw $SHIFT_FRW_COL,%mm0 # t1

    movq (%eax),%mm5   # 5 ; x0
    paddsw %mm3,%mm4 # t2 = x[2] + x[5]

    paddsw 112(%eax),%mm5   # t0 = x[0] + x[7]
    psllw $SHIFT_FRW_COL,%mm4 # t2

    movq %mm0,%mm6 # 6 ; t1
    psubsw %mm1,%mm2 # 1 ; t6 = x[1] - x[6]

    movq 8(%ebx),%mm1    # 1 ; tg_2_16
    psubsw %mm4,%mm0 # tm12 = t1 - t2

    movq 48(%eax),%mm7   # 7 ; x3
    pmulhw %mm0,%mm1 # tm12*tg_2_16

    paddsw 64(%eax),%mm7   # t3 = x[3] + x[4]
    psllw $SHIFT_FRW_COL,%mm5 # t0

    paddsw %mm4,%mm6 # 4 ; tp12 = t1 + t2
    psllw $SHIFT_FRW_COL,%mm7 # t3

    movq %mm5,%mm4 # 4 ; t0
    psubsw %mm7,%mm5 # tm03 = t0 - t3

    paddsw %mm5,%mm1 # y2 = tm03 + tm12*tg_2_16
    paddsw %mm7,%mm4 # 7 ; tp03 = t0 + t3

    por fdct_one_corr,%mm1    # correction y2 +0.5
    psllw $SHIFT_FRW_COL+1,%mm2 # t6

    pmulhw 8(%ebx),%mm5    # tm03*tg_2_16
    movq %mm4,%mm7 # 7 ; tp03

    psubsw 80(%eax),%mm3   # t5 = x[2] - x[5]
    psubsw %mm6,%mm4 # y4 = tp03 - tp12

    movq %mm1,40(%ecx)   # 1 ; save y2
    paddsw %mm6,%mm7 # 6 ; y0 = tp03 + tp12

    movq 48(%eax),%mm1   # 1 ; x3
    psllw $SHIFT_FRW_COL+1,%mm3 # t5

    psubsw 64(%eax),%mm1   # t4 = x[3] - x[4]
    movq %mm2,%mm6 # 6 ; t6

    movq %mm4,72(%ecx)   # 4 ; save y4
    paddsw %mm3,%mm2 # t6 + t5

    pmulhw 32(%ebx),%mm2    # tp65 = (t6 + t5)*cos_4_16
    psubsw %mm3,%mm6 # 3 ; t6 - t5

    pmulhw 32(%ebx),%mm6    # tm65 = (t6 - t5)*cos_4_16
    psubsw %mm0,%mm5 # 0 ; y6 = tm03*tg_2_16 - tm12

    por fdct_one_corr,%mm5    # correction y6 +0.5
    psllw $SHIFT_FRW_COL,%mm1 # t4

    por fdct_one_corr,%mm2    # correction tp65 +0.5
    movq %mm1,%mm4 # 4 ; t4

    movq (%eax),%mm3   # 3 ; x0
    paddsw %mm6,%mm1 # tp465 = t4 + tm65

    psubsw 112(%eax),%mm3   # t7 = x[0] - x[7]
    psubsw %mm6,%mm4 # 6 ; tm465 = t4 - tm65

    movq (%ebx),%mm0    # 0 ; tg_1_16
    psllw $SHIFT_FRW_COL,%mm3 # t7

    movq 16(%ebx),%mm6    # 6 ; tg_3_16
    pmulhw %mm1,%mm0 # tp465*tg_1_16

    movq %mm7,8(%ecx)   # 7 ; save y0
    pmulhw %mm4,%mm6 # tm465*tg_3_16

    movq %mm5,104(%ecx)   # 5 ; save y6
    movq %mm3,%mm7 # 7 ; t7

    movq 16(%ebx),%mm5    # 5 ; tg_3_16
    psubsw %mm2,%mm7 # tm765 = t7 - tp65

    paddsw %mm2,%mm3 # 2 ; tp765 = t7 + tp65
    pmulhw %mm7,%mm5 # tm765*tg_3_16

    paddsw %mm3,%mm0 # y1 = tp765 + tp465*tg_1_16
    paddsw %mm4,%mm6 # tm465*tg_3_16

    pmulhw (%ebx),%mm3    # tp765*tg_1_16
    ##

    por fdct_one_corr,%mm0   # correction y1 +0.5
    paddsw %mm7,%mm5 # tm765*tg_3_16

    psubsw %mm6,%mm7 # 6 ; y3 = tm765 - tm465*tg_3_16
    ##

    movq %mm0,24(%ecx)   # 0 ; save y1
    paddsw %mm4,%mm5 # 4 ; y5 = tm765*tg_3_16 + tm465

    movq %mm7,56(%ecx)   # 7 ; save y3
    psubsw %mm1,%mm3 # 1 ; y7 = tp765*tg_1_16 - tp465

    movq %mm5,88(%ecx)   # 5 ; save y5

    movq %mm3,120(%ecx)   # 3 ; save y7

#    emms;
#    }   ; end of forward_dct_col07() 
    #  done with dct_row transform


  # fdct_mmx32_cols() --
  # the following subroutine repeats the row-transform operation, 
  # except with different shift&round constants.  This version
  # does NOT transpose the output again.  Thus the final output
  # is transposed with respect to the source.
  #
  #  The output is stored into blk[], which destroys the original
  #  input data.
        mov 8(%ebp), %eax
        movl $0x08,%edi # ;x = 8

        lea tab_frw_01234567, %ebx
        mov %ecx, %eax

        lea fdct_r_row, %edx
        # for ( x = 8; x > 0; --x )  ; transform one row per iteration

# ---------- loop begin
  lp_mmx_fdct_row1: 
    movd 12(%eax),%mm5   # ; mm5 = 7 6

    punpcklwd 8(%eax),%mm5    # mm5 =  5 7 4 6

    movq %mm5,%mm2 #     ; mm2 = 5 7 4 6
    psrlq $32,%mm5 #     ; mm5 = _ _ 5 7

    movq (%eax), %mm0   # ; mm0 = 3 2 1 0
    punpcklwd %mm2,%mm5 ## mm5 = 4 5 6 7

    movq %mm0,%mm1 #     ; mm1 = 3 2 1 0
    paddsw %mm5,%mm0 #   ; mm0 = [3+4, 2+5, 1+6, 0+7] (xt3, xt2, xt1, xt0)

    psubsw %mm5,%mm1 #   ; mm1 = [3-4, 2-5, 1-6, 0-7] (xt7, xt6, xt5, xt4)
    movq %mm0,%mm2 #     ; mm2 = [ xt3 xt2 xt1 xt0 ]

    #movq [ xt3xt2xt1xt0 ], mm0;
    #movq [ xt7xt6xt5xt4 ], mm1;

    punpcklwd %mm1,%mm0 ## mm0 = [ xt5 xt1 xt4 xt0 ]

    punpckhwd %mm1,%mm2 ## mm2 = [ xt7 xt3 xt6 xt2 ]
    movq %mm2,%mm1 #     ; mm1

    ## shuffle bytes around

#  movq mm0,  [INP] ; 0 ; x3 x2 x1 x0

#  movq mm1,  [INP+8] ; 1 ; x7 x6 x5 x4
    movq %mm0,%mm2 # 2 ; x3 x2 x1 x0

    movq (%ebx),%mm3    # 3 ; w06 w04 w02 w00
    punpcklwd %mm1,%mm0 # x5 x1 x4 x0

    movq %mm0,%mm5 # 5 ; x5 x1 x4 x0
    punpckldq %mm0,%mm0 # x4 x0 x4 x0  [ xt2 xt0 xt2 xt0 ]

    movq 8(%ebx),%mm4    # 4 ; w07 w05 w03 w01
    punpckhwd %mm1,%mm2 # 1 ; x7 x3 x6 x2

    pmaddwd %mm0,%mm3 # x4*w06+x0*w04 x4*w02+x0*w00
    movq %mm2,%mm6 # 6 ; x7 x3 x6 x2

    movq 32(%ebx),%mm1    # 1 ; w22 w20 w18 w16
    punpckldq %mm2,%mm2 # x6 x2 x6 x2  [ xt3 xt1 xt3 xt1 ]

    pmaddwd %mm2,%mm4 # x6*w07+x2*w05 x6*w03+x2*w01
    punpckhdq %mm5,%mm5 # x5 x1 x5 x1  [ xt6 xt4 xt6 xt4 ]

    pmaddwd 16(%ebx),%mm0    # x4*w14+x0*w12 x4*w10+x0*w08
    punpckhdq %mm6,%mm6 # x7 x3 x7 x3  [ xt7 xt5 xt7 xt5 ]

    movq 40(%ebx),%mm7    # 7 ; w23 w21 w19 w17
    pmaddwd %mm5,%mm1 # x5*w22+x1*w20 x5*w18+x1*w16
#mm3 = a1, a0 (y2,y0)
#mm1 = b1, b0 (y3,y1)
#mm0 = a3,a2  (y6,y4)
#mm5 = b3,b2  (y7,y5)

    paddd (%edx),%mm3    # +rounder (y2,y0)
    pmaddwd %mm6,%mm7 # x7*w23+x3*w21 x7*w19+x3*w17

    pmaddwd 24(%ebx),%mm2    # x6*w15+x2*w13 x6*w11+x2*w09
    paddd %mm4,%mm3 # 4 ; a1=sum(even1) a0=sum(even0) ; now ( y2, y0)

    pmaddwd 48(%ebx),%mm5    # x5*w30+x1*w28 x5*w26+x1*w24
    ##

    pmaddwd 56(%ebx),%mm6    # x7*w31+x3*w29 x7*w27+x3*w25
    paddd %mm7,%mm1 # 7 ; b1=sum(odd1) b0=sum(odd0) ; now ( y3, y1)

    paddd (%edx),%mm0    # +rounder (y6,y4)
    psrad $SHIFT_FRW_ROW,%mm3 # (y2, y0)

    paddd (%edx),%mm1    # +rounder (y3,y1)
    paddd %mm2,%mm0 # 2 ; a3=sum(even3) a2=sum(even2) ; now (y6, y4)

    paddd (%edx),%mm5    # +rounder (y7,y5)
    psrad $SHIFT_FRW_ROW,%mm1 # y1=a1+b1 y0=a0+b0

    paddd %mm6,%mm5 # 6 ; b3=sum(odd3) b2=sum(odd2) ; now ( y7, y5)
    psrad $SHIFT_FRW_ROW,%mm0 #y3=a3+b3 y2=a2+b2

    add $10,%ecx
    psrad $SHIFT_FRW_ROW,%mm5 # y4=a3-b3 y5=a2-b2

    add $10,%eax
    packssdw %mm0,%mm3 # 0 ; y6 y4 y2 y0

    packssdw %mm5,%mm1 # 3 ; y7 y5 y3 y1
    movq %mm3,%mm6 #    ; mm0 = y6 y4 y2 y0

    punpcklwd %mm1,%mm3 # ; y3 y2 y1 y0
    subl $0x01,%edi #   ; i = i - 1

    punpckhwd %mm1,%mm6 # ; y7 y6 y5 y4
    add $40,%ebx

    movq  %mm3,-16(%ecx)   # 1 ; save y3 y2 y1 y0

    movq  %mm6,-8(%ecx)   # 7 ; save y7 y6 y5 y4

    cmpl $0x00,%edi #
    jg lp_mmx_fdct_row1     #  ; begin fdct processing on next row
                ## 
                ## Tidy up and return
                ##
        popl %edi
        popl %edx
        popl %ecx
        popl %ebx

        popl %ebp               # restore stack pointer
        emms
        ret

