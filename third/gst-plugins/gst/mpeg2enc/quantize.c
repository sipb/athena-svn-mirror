/* quantize.c, quantization / inverse quantization                          */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpeg2enc.h"

static void iquant1_intra _ANSI_ARGS_((short *src, short *dst,
  int dc_prec, unsigned short *quant_mat, int mquant));
static void iquant1_non_intra _ANSI_ARGS_((short *src, short *dst,
  unsigned short *quant_mat, int mquant));

  /* Unpredictable branches suck on modern CPU's... */
#define fabsshift ((8*sizeof(unsigned int))-1)
#define fastabs(x) (((x)-(((unsigned int)(x))>>fabsshift)) ^ ((x)>>fabsshift))
#define signmask(x) (((int)x)>>fabsshift)
#define samesign(x,y) (y+(signmask(x) & -(y<<1)))

/*
 * Quantisation matrix weighted Coefficient sum fixed-point
 * integer with low 16 bits fractional...
 * To be used for rate control as a measure of dct block
 * complexity...
 *
 */
int (*quant_weight_coeff_sum)( short *blk, unsigned short * i_quant_mat );

static int quant_weight_coeff_sum_C( short *blk, unsigned short * i_quant_mat );
#ifdef HAVE_CPU_I386
extern int quant_weight_coeff_sum_mmx( short *blk, unsigned short * i_quant_mat );
#endif

void init_quant(vid_stream) 
mpeg2enc_vid_stream *vid_stream;
{
  quant_weight_coeff_sum = quant_weight_coeff_sum_C;
#ifdef HAVE_CPU_I386
  if (gst_cpu_get_flags() & GST_CPU_FLAG_MMX) 
    quant_weight_coeff_sum = quant_weight_coeff_sum_mmx;
#endif
}

static int quant_weight_coeff_sum_C( short *blk, unsigned short * i_quant_mat )
{
  int i;
  int sum = 0;
  for( i = 0; i < 64; i+=2 )
  {
    sum += abs((int)blk[i]) * (i_quant_mat[i]) + abs((int)blk[i+1]) * (i_quant_mat[i+1]);
  }
  return sum;
 /* In case you're wondering typical average coeff_sum's for a rather noisy video
  *         are around 20.0.
  *                 */
}

/* Test Model 5 quantization
 *
 * this quantizer has a bias of 1/8 stepsize towards zero
 * (except for the DC coefficient)
 */
int quant_intra(vid_stream,src,dst,dc_prec,quant_mat,mquant)
mpeg2enc_vid_stream *vid_stream;
short *src, *dst;
int dc_prec;
unsigned short *quant_mat;
int mquant;
{
  int i;
  int x, y, d;
  int clipvalue = vid_stream->mpeg1 ? 255 : 2047;

  x = src[0];
  d = 8>>dc_prec; /* intra_dc_mult */
  dst[0] = (x>=0) ? (x+(d>>1))/d : -((-x+(d>>1))/d); /* round(x/d) */

  for (i=1; i<64; i++)
  {
    x = src[i];
    d = quant_mat[i];
    y = (32*fastabs(x) + (d>>1) + d*((3*mquant+2)>>2))/(quant_mat[i]*2*mquant);
    if (y > clipvalue)
    {
      y=clipvalue;
    }

    dst[i] = samesign(x,y);
  }

  return 1;
}

int quant_non_intra(vid_stream,src,dst,quant_mat,mquant)
mpeg2enc_vid_stream *vid_stream;
short *src, *dst;
unsigned short *quant_mat;
int mquant;
{
  int i;
  int x, y, d;
  int nzflag;
  int clipvalue = vid_stream->mpeg1 ? 255 : 2047;

  nzflag = 0;

  for (i=0; i<64; i++)
  {
    x = abs(src[i]);
    d = quant_mat[i];
    y = (32*x + (d>>1))/(d*2*mquant); /* round(32*x/quant_mat) */

    /* clip to syntax limits */
    if (y > clipvalue)
      y = clipvalue;

    nzflag |= (dst[i] = samesign(src[i],y));
  }

  return !!nzflag;
}

/* MPEG-2 inverse quantization */
void iquant_intra(vid_stream,src,dst,dc_prec,quant_mat,mquant)
mpeg2enc_vid_stream *vid_stream;
short *src, *dst;
int dc_prec;
unsigned short *quant_mat;
int mquant;
{
  int i, val, sum;

  if (vid_stream->mpeg1)
    iquant1_intra(src,dst,dc_prec,quant_mat,mquant);
  else
  {
    sum = dst[0] = src[0] << (3-dc_prec);
    for (i=1; i<64; i++)
    {
      val = (int)(src[i]*quant_mat[i]*mquant)/16;
      sum+= dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
    }

    /* mismatch control */
    if ((sum&1)==0)
      dst[63]^= 1;
  }
}

void iquant_non_intra(vid_stream,src,dst,quant_mat,mquant)
mpeg2enc_vid_stream *vid_stream;
short *src, *dst;
unsigned short *quant_mat;
int mquant;
{
  int i, val, sum;

  if (vid_stream->mpeg1)
    iquant1_non_intra(src,dst,quant_mat,mquant);
  else
  {
    sum = 0;
    for (i=0; i<64; i++)
    {
      val = src[i];
      if (val!=0)
        val = (int)((2*val+(val>0 ? 1 : -1))*quant_mat[i]*mquant)/32;
      sum+= dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
    }

    /* mismatch control */
    if ((sum&1)==0)
      dst[63]^= 1;
  }
}

/* MPEG-1 inverse quantization */
static void iquant1_intra(src,dst,dc_prec,quant_mat,mquant)
short *src, *dst;
int dc_prec;
unsigned short *quant_mat;
int mquant;
{
  int i, val;

  dst[0] = src[0] << (3-dc_prec);
  for (i=1; i<64; i++)
  {
    val = (int)(src[i]*quant_mat[i]*mquant)/16;

    /* mismatch control */
    if ((val&1)==0 && val!=0)
      val+= (val>0) ? -1 : 1;

    /* saturation */
    dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
  }
}

static void iquant1_non_intra(src,dst,quant_mat,mquant)
short *src, *dst;
unsigned short *quant_mat;
int mquant;
{
  int i, val;

  for (i=0; i<64; i++)
  {
    val = src[i];
    if (val!=0)
    {
      val = (int)((2*val+(val>0 ? 1 : -1))*quant_mat[i]*mquant)/32;

      /* mismatch control */
      if ((val&1)==0 && val!=0)
        val+= (val>0) ? -1 : 1;
    }

    /* saturation */
    dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
  }
}
