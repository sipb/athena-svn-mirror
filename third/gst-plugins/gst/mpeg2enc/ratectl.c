/* ratectl.c, bitrate control routines (linear quantization only currently) */

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
#include <math.h>
#include <stdlib.h>

#include "mpeg2enc.h"

/* private prototypes */
static double calc_actj _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *frame));

/* rate control variables */
int Xi, Xp, Xb, r, d0i, d0p, d0b;
double avg_act, sum_actj;
static int R, T, d;
static double actsum;
static int Np, Nb, S, Q;
static int prev_mquant;

static double Ki = 1.0;
static double Kp = 1.0;  
static double Kb = 1.4; 

void rc_init_seq(mpeg2enc_vid_stream *vid_stream)
{
  /* reaction parameter (constant) */
  if (r==0)  r = (int)floor(2.0*vid_stream->seq.bit_rate/vid_stream->seq.frame_rate + 0.5);

  /* average activity */
  if (avg_act==0.0)  avg_act = 400.0;

  /* remaining # of bits in GOP */
  R = 0;

  /* global complexity measure */
  if (Xi==0) Xi = (int)floor(160.0*vid_stream->seq.bit_rate/115.0 + 0.5);
  if (Xp==0) Xp = (int)floor( 60.0*vid_stream->seq.bit_rate/115.0 + 0.5);
  if (Xb==0) Xb = (int)floor( 42.0*vid_stream->seq.bit_rate/115.0 + 0.5);

  if (d0i==0) d0i = (int)floor(Ki*r*0.15 + 0.5);
  if (d0p==0) d0p = (int)floor(Kp*r*0.15 + 0.5);
  if (d0b==0) d0b = (int)floor(Kb*r*0.15 + 0.5);
  /* virtual buffer fullness */
  /*
  if (d0i==0) d0i = (int)floor(10.0*r/31.0 + 0.5);
  if (d0p==0) d0p = (int)floor(10.0*r/31.0 + 0.5);
  if (d0b==0) d0b = (int)floor(1.4*10.0*r/31.0 + 0.5);
  */
/*
  if (d0i==0) d0i = (int)floor(10.0*r/(qscale_tab[0] ? 56.0 : 31.0) + 0.5);
  if (d0p==0) d0p = (int)floor(10.0*r/(qscale_tab[1] ? 56.0 : 31.0) + 0.5);
  if (d0b==0) d0b = (int)floor(1.4*10.0*r/(qscale_tab[2] ? 56.0 : 31.0) + 0.5);
*/

  if (!vid_stream->quiet) {
    fprintf(stdout,"\nrate control: sequence initialization\n");
    fprintf(stdout,
      " initial global complexity measures (I,P,B): Xi=%d, Xp=%d, Xb=%d\n",
      Xi, Xp, Xb);
    fprintf(stdout," reaction parameter: r=%d\n", r);
    fprintf(stdout,
      " initial virtual buffer fullness (I,P,B): d0i=%d, d0p=%d, d0b=%d\n",
      d0i, d0p, d0b);
    fprintf(stdout," initial average activity: avg_act=%.1f\n", avg_act);
  }
}

void rc_init_GOP(vid_stream,np,nb)
mpeg2enc_vid_stream *vid_stream;
int np,nb;
{
  R += (int) floor((1 + np + nb) * vid_stream->seq.bit_rate / vid_stream->seq.frame_rate + 0.5);
  Np = vid_stream->fieldpic ? 2*np+1 : np;
  Nb = vid_stream->fieldpic ? 2*nb : nb;

  if (!vid_stream->quiet) {
    fprintf(stdout,"\nrate control: new group of pictures (GOP) %g %g\n", vid_stream->seq.bit_rate, vid_stream->seq.frame_rate);
    fprintf(stdout," target number of bits for GOP: R=%d\n",R);
    fprintf(stdout," number of P pictures in GOP: Np=%d\n",Np);
    fprintf(stdout," number of B pictures in GOP: Nb=%d\n",Nb);
  }
}

/* Note: we need to substitute K for the 1.4 and 1.0 constants -- this can
   be modified to fit image content */

/* Step 1: compute target bits for current picture being coded */
void rc_init_pict(vid_stream,frame)
mpeg2enc_vid_stream *vid_stream;
unsigned char *frame;
{
  double Tmin;

  switch (vid_stream->picture.pict_type)
  {
  case I_TYPE:
    T = (int) floor(R/(1.0+Np*Xp*Ki/(Xi*Kp)+Nb*Xb*Ki/(Xi*Kb)) + 0.5);
    d = d0i;
    break;
  case P_TYPE:
    T = (int) floor(R/(Np+Nb*Kp*Xb/(Kb*Xp)) + 0.5);
    d = d0p;
    break;
  case B_TYPE:
    T = (int) floor(R/(Nb+Np*Kb*Xp/(Kp*Xb)) + 0.5);
    d = d0b;
    break;
  }

  Tmin = (int) floor(vid_stream->seq.bit_rate/(8.0*vid_stream->seq.frame_rate) + 0.5);

  if (T<Tmin)
    T = Tmin;

  S = gst_putbits_bitcount(&vid_stream->pb);
  /*printf("rate control: start of picture S=%d\n", S);*/
  Q = 0;

  sum_actj = calc_actj(vid_stream,frame);
  avg_act = sum_actj/(vid_stream->seq.mb_width*vid_stream->seq.mb_height2);
  actsum = 0.0;


  if (!vid_stream->quiet) {
    fprintf(stdout,"\nrate control: start of picture S=%d type=%d\n", S, vid_stream->picture.pict_type);
    fprintf(stdout," target number of bits: T=%d\n",T);
  }
}


static double calc_actj(vid_stream,frame)
mpeg2enc_vid_stream *vid_stream;
unsigned char *frame;
{
  int i,j,k;
  double actj, sum;
  unsigned short *i_q_mat;

  k = 0;
  sum = 0.0;

  for (j=0; j<vid_stream->seq.height2; j+=16) 
  {
    for (i=0; i<vid_stream->seq.width; i+=16)
    {
        if( vid_stream->mbinfo[k].mb_type  & MB_INTRA )
        {
          i_q_mat = vid_stream->i_intra_q;
        }
        else
        {
          i_q_mat = vid_stream->i_inter_q;
        }

        actj  = (double)quant_weight_coeff_sum( &vid_stream->mbinfo[k].dctblocks[0], i_q_mat ) /
                       (double) COEFFSUM_SCALE;
     /*
     actj += (double)quant_weight_coeff_sum( &vid_stream->mbinfo[k].dctblocks[1], i_q_mat ) /
                       (double) COEFFSUM_SCALE         ;
     actj += (double) quant_weight_coeff_sum( &vid_stream->mbinfo[k].dctblocks[0], i_q_mat ) /
                       (double) COEFFSUM_SCALE  ;
     actj += (double) quant_weight_coeff_sum( &vid_stream->mbinfo[k].dctblocks[1], i_q_mat ) /
                       (double) COEFFSUM_SCALE  ;
		       */
     /*actj *=4;*/


      sum += actj;

      /*printf("calc actj %f %d\n", actj, vid_stream->mbinfo[k].dctblocks[0] );*/
      vid_stream->mbinfo[k].act = actj;

      k++;
    }
  }

  return sum;
}

void rc_update_pict(vid_stream)
mpeg2enc_vid_stream *vid_stream;
{
  double X, P, percent, minPercent;
  unsigned int i;

#if 1
  minPercent = 0.75;

  P = gst_putbits_bitcount(&vid_stream->pb) - S; /* total # of bits in picture */
  /*printf("rate control: end of picture P=%lf S=%d %d\n", P, S, T);*/
  percent = P / T;
  if (percent > 0 && percent < minPercent) /* pad some bits if actual is minPercent of target bits */
  {
    P = (T * 0.90) - P;
    P -= ((unsigned int)P % 32);
    /*printf("rate control: stuffing P=%d %g\n", (unsigned int)P, percent);*/
    for (i = 0; i < (unsigned int)P; i += 32) {
      gst_putbits(&vid_stream->pb,0, 32);
    }
    gst_putbits_align(&vid_stream->pb);
    S = gst_putbits_bitcount(&vid_stream->pb) - S;
  }
  else
  {
    S = P;
    /*printf("rate control: end of picture P=%lf S=%d\n", P, S);*/
  }
#else
  S = gst_putbits_bitcount(&vid_stream->pb) - S; /* total # of bits in picture */
#endif

  R-= S; /* remaining # of bits in GOP */
  /*printf("rate control: remaining bits in GOP r=%d\n", R);*/
  X = (int) floor(S*((0.5*(double)Q)/(vid_stream->seq.mb_width*vid_stream->seq.mb_height2)) + 0.5);
  d+= S - T;

  switch (vid_stream->picture.pict_type)
  {
  case I_TYPE:
    Xi = X;
    d0i = d;
    break;
  case P_TYPE:
    Xp = X;
    d0p = d;
    Np--;
    break;
  case B_TYPE:
    Xb = X;
    d0b = d;
    Nb--;
    break;
  }

  if (!vid_stream->quiet) {
    fprintf(stdout,"\nrate control: end of picture\n");
    fprintf(stdout," actual number of bits: S=%d\n",S);
    fprintf(stdout," average quantization parameter Q=%.1f\n",
      (double)Q/(vid_stream->seq.mb_width*vid_stream->seq.mb_height2));
    fprintf(stdout," remaining number of bits in GOP: R=%d\n",R);
    fprintf(stdout,
      " global complexity measures (I,P,B): Xi=%d, Xp=%d, Xb=%d\n",
      Xi, Xp, Xb);
    fprintf(stdout,
      " virtual buffer fullness (I,P,B): d0i=%d, d0p=%d, d0b=%d\n",
      d0i, d0p, d0b);
    fprintf(stdout," remaining number of P pictures in GOP: Np=%d\n",Np);
    fprintf(stdout," remaining number of B pictures in GOP: Nb=%d\n",Nb);
    fprintf(stdout," average activity: avg_act=%.1f\n", avg_act);
  }
}

/* compute initial quantization stepsize (at the beginning of picture) */
int rc_start_mb(vid_stream)
mpeg2enc_vid_stream *vid_stream;
{
  int mquant;

  if (vid_stream->picture.q_scale_type)
  {
    mquant = (int) floor(2.0*d*31.0/r + 0.5);

    /* clip mquant to legal (linear) range */
    if (mquant<1)
      mquant = 1;
    if (mquant>112)
      mquant = 112;

    /* map to legal quantization level */
    mquant = non_linear_mquant_table[map_non_linear_mquant[mquant]];
  }
  else
  {
    mquant = (int) floor(d*31.0/r + 0.5);
    mquant <<= 1;

    /* clip mquant to legal (linear) range */
    if (mquant<2)
      mquant = 2;
    if (mquant>62)
      mquant = 62;

    prev_mquant = mquant;
  }

  /*fprintf(stdout,"rc_start_mb:\n");*/
  /*fprintf(stdout,"mquant=%d\n",mquant);*/

  return mquant;
}

/* Step 2: measure virtual buffer - estimated buffer discrepancy */
int rc_calc_mquant(vid_stream,j,seq)
mpeg2enc_vid_stream *vid_stream;
int j;
int seq;
{
  int mquant;
  double dj, Qj, actj, N_actj;

  /* measure virtual buffer discrepancy from uniform distribution model */
  dj = d + (gst_putbits_bitcount(&vid_stream->pb)-S) - actsum * T / sum_actj;

  /*printf("%d %d %d %f %f %f\n", d, T, S, dj, (sum_actj-actsum)/(sum_actj), actsum);*/
  /*printf("%d %d %f \n", T, S, dj);*/

  /* scale against dynamic range of mquant and the bits/picture count */
  Qj = dj*31.0/r;
/*Qj = dj*(q_scale_type ? 56.0 : 31.0)/r;  */

  actj = vid_stream->mbinfo[j].act;
  actsum+= actj;

  /*avg_act = (actsum)/seq;*/
  /* compute normalized activity */
  /*if (vid_stream->picture.pict_type == I_TYPE) { */
    N_actj =  actj < avg_act ? 1.0 : (2.0*actj + avg_act)/(actj +  2.0*avg_act);
  /* } */
  /*else { */
   /* N_actj = (actj+2.0*avg_act) / (2.0*actj+avg_act); */
  /*} */

  if (vid_stream->picture.q_scale_type)
  {
    /* modulate mquant with combined buffer and local activity measures */
    mquant = (int) floor(2.0*Qj*N_actj + 0.5);

    /* clip mquant to legal (linear) range */
    if (mquant<1)
      mquant = 1;
    if (mquant>112)
      mquant = 112;

    /* map to legal quantization level */
    mquant = non_linear_mquant_table[map_non_linear_mquant[mquant]];
  }
  else
  {
    /* modulate mquant with combined buffer and local activity measures */
    mquant = (int) floor(Qj*N_actj + 0.5);
    /*printf("%d, %f, %f, %d %f\n", j, actj, N_actj, mquant, Qj); */
    mquant <<= 1;

    /* clip mquant to legal (linear) range */
    if (mquant<2)
      mquant = 2;
    if (mquant>62)
      mquant = 62;

    /* ignore small changes in mquant */
    if (mquant>=8 && (mquant-prev_mquant)>=-4 && (mquant-prev_mquant)<=4)
      mquant = prev_mquant;

    prev_mquant = mquant;
  }

  Q+= mquant; /* for calculation of average mquant */

  /*fprintf(stdout,"rc_calc_mquant(%d): ",j); */
  /*fprintf(stdout,"dj=%g, Qj=%g, actj=%g, N_actj=%g, mquant=%d\n",*/
  /*  dj,Qj,actj,N_actj,mquant); */

  return mquant;
}

/* VBV calculations
 *
 * generates warnings if underflow or overflow occurs
 */

/* vbv_end_of_picture
 *
 * - has to be called directly after writing picture_data()
 * - needed for accurate VBV buffer overflow calculation
 * - assumes there is no byte stuffing prior to the next start code
 */

static int bitcnt_EOP;

void vbv_end_of_picture(vid_stream)
mpeg2enc_vid_stream *vid_stream;
{
  bitcnt_EOP = gst_putbits_bitcount(&vid_stream->pb);
  bitcnt_EOP = (bitcnt_EOP + 7) & ~7; /* account for bit stuffing */
}

/* calc_vbv_delay
 *
 * has to be called directly after writing the picture start code, the
 * reference point for vbv_delay
 */

void calc_vbv_delay(vid_stream)
mpeg2enc_vid_stream *vid_stream;
{
  double picture_delay;
  static double next_ip_delay; /* due to frame reordering delay */
  static double decoding_time;

  /* number of 1/90000 s ticks until next picture is to be decoded */
  if (vid_stream->picture.pict_type == B_TYPE)
  {
    if (vid_stream->seq.prog_seq)
    {
      if (!vid_stream->picture.repeatfirst)
        picture_delay = 90000.0/vid_stream->seq.frame_rate; /* 1 frame */
      else
      {
        if (!vid_stream->picture.topfirst)
          picture_delay = 90000.0*2.0/vid_stream->seq.frame_rate; /* 2 frames */
        else
          picture_delay = 90000.0*3.0/vid_stream->seq.frame_rate; /* 3 frames */
      }
    }
    else
    {
      /* interlaced */
      if (vid_stream->fieldpic)
        picture_delay = 90000.0/(2.0*vid_stream->seq.frame_rate); /* 1 field */
      else
      {
        if (!vid_stream->picture.repeatfirst)
          picture_delay = 90000.0*2.0/(2.0*vid_stream->seq.frame_rate); /* 2 flds */
        else
          picture_delay = 90000.0*3.0/(2.0*vid_stream->seq.frame_rate); /* 3 flds */
      }
    }
  }
  else
  {
    /* I or P picture */
    if (vid_stream->fieldpic)
    {
      if(vid_stream->picture.topfirst==(vid_stream->picture.pict_struct==TOP_FIELD))
      {
        /* first field */
        picture_delay = 90000.0/(2.0*vid_stream->seq.frame_rate);
      }
      else
      {
        /* second field */
        /* take frame reordering delay into account */
        picture_delay = next_ip_delay - 90000.0/(2.0*vid_stream->seq.frame_rate);
      }
    }
    else
    {
      /* frame picture */
      /* take frame reordering delay into account*/
      picture_delay = next_ip_delay;
    }

    if (!vid_stream->fieldpic || vid_stream->picture.topfirst!=(vid_stream->picture.pict_struct==TOP_FIELD))
    {
      /* frame picture or second field */
      if (vid_stream->seq.prog_seq)
      {
        if (!vid_stream->picture.repeatfirst)
          next_ip_delay = 90000.0/vid_stream->seq.frame_rate;
        else
        {
          if (!vid_stream->picture.topfirst)
            next_ip_delay = 90000.0*2.0/vid_stream->seq.frame_rate;
          else
            next_ip_delay = 90000.0*3.0/vid_stream->seq.frame_rate;
        }
      }
      else
      {
        if (vid_stream->fieldpic)
          next_ip_delay = 90000.0/(2.0*vid_stream->seq.frame_rate);
        else
        {
          if (!vid_stream->picture.repeatfirst)
            next_ip_delay = 90000.0*2.0/(2.0*vid_stream->seq.frame_rate);
          else
            next_ip_delay = 90000.0*3.0/(2.0*vid_stream->seq.frame_rate);
        }
      }
    }
  }

  if (decoding_time==0.0)
  {
    /* first call of calc_vbv_delay */
    /* we start with a 7/8 filled VBV buffer (12.5% back-off) */
    picture_delay = ((vid_stream->seq.vbv_buffer_size*16384*7)/8)*90000.0/vid_stream->seq.bit_rate;
    if (vid_stream->fieldpic)
      next_ip_delay = (int)(90000.0/vid_stream->seq.frame_rate+0.5);
  }

  /* VBV checks */

  /* check for underflow (previous picture) */
  if (!vid_stream->seq.low_delay && (decoding_time < bitcnt_EOP*90000.0/vid_stream->seq.bit_rate))
  {
    /* picture not completely in buffer at intended decoding time */
    if (!vid_stream->quiet)
      fprintf(stderr,"vbv_delay underflow! (decoding_time=%.1f, t_EOP=%.1f\n)",
        decoding_time, bitcnt_EOP*90000.0/vid_stream->seq.bit_rate);
  }

  /* when to decode current frame */
  decoding_time += picture_delay;

  /* warning: bitcount() may overflow (e.g. after 9 min. at 8 Mbit/s */
  vid_stream->picture.vbv_delay = (int)(decoding_time - gst_putbits_bitcount(&vid_stream->pb)*90000.0/vid_stream->seq.bit_rate);

  /* check for overflow (current picture) */
  if ((decoding_time - bitcnt_EOP*90000.0/vid_stream->seq.bit_rate)
      > (vid_stream->seq.vbv_buffer_size*16384)*90000.0/vid_stream->seq.bit_rate)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"vbv_delay overflow!\n");
  }

  if (!vid_stream->quiet) {
    fprintf(stdout,
      "\nvbv_delay=%d (bitcount=%d, decoding_time=%.2f, bitcnt_EOP=%d)\n",
      vid_stream->picture.vbv_delay,gst_putbits_bitcount(&vid_stream->pb),decoding_time,bitcnt_EOP);
  }

  if (vid_stream->picture.vbv_delay<0)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"vbv_delay underflow: %d\n",vid_stream->picture.vbv_delay);
    vid_stream->picture.vbv_delay = 0;
  }

  if (vid_stream->picture.vbv_delay>65535)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"vbv_delay overflow: %d\n",vid_stream->picture.vbv_delay);
    vid_stream->picture.vbv_delay = 65535;
  }
}
