/* putpic.c, block and motion vector encoding routines                      */

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
#include "mpeg2enc.h"

/* private prototypes */
static void putmvs _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int MV[2][2][2], int PMV[2][2][2],
  int mv_field_sel[2][2], int dmvector[2], int s, int motion_type,
  int hor_f_code, int vert_f_code));

void putpict_real(mpeg2enc_vid_stream *vid_stream, unsigned char *frame, int real);

void putpict(vid_stream,frame)
mpeg2enc_vid_stream *vid_stream;
unsigned char *frame;
{
  /*
  int i, j;
  gst_putbits_t temp = vid_stream->pb;
  int info_blocks, blocks;
  mpeg2enc_vid_stream tempvs = *vid_stream;

  info_blocks = vid_stream->seq.mb_width*vid_stream->seq.mb_height2;
  blocks = vid_stream->seq.mb_width*vid_stream->seq.mb_height2*vid_stream->seq.block_count;

  for (i=0; i< blocks;i++) {
    for (j=0; j< 64;j++) {
      vid_stream->blocks_temp[i][j] = vid_stream->blocks[i][j];
    }
  }
  for (i=0; i< info_blocks;i++) {
    vid_stream->mbinfo_temp[i] = vid_stream->mbinfo[i];
  }

  rc_init_pict(vid_stream, frame); * set up rate control  *

  gst_putbits_new_empty_buffer(&vid_stream->pb, 1000000);
  putpict_real(vid_stream, frame, 0);

  *vid_stream = tempvs;

  for (i=0; i< blocks;i++) {
    for (j=0; j< 64;j++) {
      vid_stream->blocks[i][j] =vid_stream->blocks_temp[i][j];
    }
  }
  for (i=0; i< info_blocks;i++) {
    *vid_stream->mbinfo[i] =vid_stream->mbinfo_temp[i]; *
  }
  vid_stream->pb = temp;
  */

  putpict_real(vid_stream, frame, 2);
}

/* quantization / variable length encoding of a complete picture */
void putpict_real(vid_stream,frame,real)
mpeg2enc_vid_stream *vid_stream;
unsigned char *frame;
int real;
{
  int i, j, k, kold,comp, cc;
  int mb_type;
  int PMV[2][2][2];
  int prev_mquant;
  int cbp, MBAinc=0, MBPS, MBPSinc = -1;

  MBPS = vid_stream->MBPS;

  if (real != 1) rc_init_pict(vid_stream, frame); /* set up rate control */

  /* picture header and picture coding extension */
  putpicthdr(vid_stream);

  if (!vid_stream->mpeg1)
    putpictcodext(vid_stream);

  prev_mquant = rc_start_mb(vid_stream); /* initialize quantization parameter */

  kold = 0;

  for (j=0; j<vid_stream->seq.mb_height2; j++)
  {
    /* macroblock row loop */

    for (i=0; i<vid_stream->seq.mb_width; i++)
    {
      MBPSinc++;
      if (MBPSinc > MBPS) MBPSinc = 0;

      if (!real) {
        k = kold;
      }
      else k= kold;
      /* macroblock loop */
      /*if (i==0)	 */
      if (MBPSinc==0)
      {
        /* slice header (6.2.4) */
        gst_putbits_align(&vid_stream->pb);

        if (vid_stream->mpeg1 || vid_stream->seq.vertical_size<=2800)
          gst_putbits(&vid_stream->pb,SLICE_MIN_START+j,32); /* slice_start_code */
        else
        {
          gst_putbits(&vid_stream->pb,SLICE_MIN_START+(j&127),32); /* slice_start_code */
          gst_putbits(&vid_stream->pb,j>>7,3); /* slice_vertical_position_extension */
        }
  
        /* quantiser_scale_code */
        gst_putbits(&vid_stream->pb,vid_stream->picture.q_scale_type ? map_non_linear_mquant[prev_mquant]
                             : prev_mquant >> 1, 5);
  
        gst_putbits(&vid_stream->pb,0,1); /* extra_bit_slice */
  
        /* reset predictors */

        for (cc=0; cc<3; cc++) {
          vid_stream->dc_dct_pred[cc] = 0;
	}

        PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
        PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
  
        MBAinc = i + 1; /* first MBAinc denotes absolute position */
      }

      mb_type = vid_stream->mbinfo[k].mb_type;

      if (real != 1) {
        /* determine mquant (rate control) */
        vid_stream->mbinfo[k].mquant = rc_calc_mquant(vid_stream, k, kold);
      }

      /* quantize macroblock */
      if (mb_type & MB_INTRA)
      {
        for (comp=0; comp<vid_stream->seq.block_count; comp++)
          quant_intra(vid_stream, vid_stream->blocks[k*vid_stream->seq.block_count+comp],vid_stream->blocks[k*vid_stream->seq.block_count+comp],
                      vid_stream->picture.dc_prec,vid_stream->intra_q,vid_stream->mbinfo[k].mquant);
        vid_stream->mbinfo[k].cbp = cbp = (1<<vid_stream->seq.block_count) - 1;
      }
      else
      {
        cbp = 0;
        for (comp=0;comp<vid_stream->seq.block_count;comp++)
          cbp = (cbp<<1) | quant_non_intra(vid_stream, vid_stream->blocks[k*vid_stream->seq.block_count+comp],
                                           vid_stream->blocks[k*vid_stream->seq.block_count+comp],
                                           vid_stream->inter_q,vid_stream->mbinfo[k].mquant);

        vid_stream->mbinfo[k].cbp = cbp;

        if (cbp)
          mb_type|= MB_PATTERN;
      }

      /* output mquant if it has changed */
      if (cbp && prev_mquant!=vid_stream->mbinfo[k].mquant)
        mb_type|= MB_QUANT;

      /* check if macroblock can be skipped */
      /*if (i!=0 && i!= vid_stream->seq.mb_width-1 && !cbp) */
      if (MBPSinc!=0 && MBPSinc!= MBPS-1 && !cbp)
      {
        /* no DCT coefficients and neither first nor last macroblock of slice */

        if (vid_stream->picture.pict_type==P_TYPE && !(mb_type&MB_FORWARD))
        {
          /* P picture, no motion vectors -> skip */

          /* reset predictors */

          for (cc=0; cc<3; cc++)
            vid_stream->dc_dct_pred[cc] = 0;

          PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
          PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;

          vid_stream->mbinfo[k].mb_type = mb_type;
          vid_stream->mbinfo[k].skipped = 1;
          MBAinc++;
          kold++;
          continue;
        }

        if (vid_stream->picture.pict_type==B_TYPE && vid_stream->picture.pict_struct==FRAME_PICTURE
            && vid_stream->mbinfo[k].motion_type==MC_FRAME
            && ((vid_stream->mbinfo[k-1].mb_type^mb_type)&(MB_FORWARD|MB_BACKWARD))==0
            && (!(mb_type&MB_FORWARD) ||
                (PMV[0][0][0]==vid_stream->mbinfo[k].MV[0][0][0] &&
                 PMV[0][0][1]==vid_stream->mbinfo[k].MV[0][0][1]))
            && (!(mb_type&MB_BACKWARD) ||
                (PMV[0][1][0]==vid_stream->mbinfo[k].MV[0][1][0] &&
                 PMV[0][1][1]==vid_stream->mbinfo[k].MV[0][1][1])))
        {
          /* conditions for skipping in B frame pictures:
           * - must be frame predicted
           * - must be the same prediction type (forward/backward/interp.)
           *   as previous macroblock
           * - relevant vectors (forward/backward/both) have to be the same
           *   as in previous macroblock
           */

          vid_stream->mbinfo[k].mb_type = mb_type;
          vid_stream->mbinfo[k].skipped = 1;
          MBAinc++;
          kold++;
          continue;
        }

        if (vid_stream->picture.pict_type==B_TYPE && vid_stream->picture.pict_struct!=FRAME_PICTURE
            && vid_stream->mbinfo[k].motion_type==MC_FIELD
            && ((vid_stream->mbinfo[k-1].mb_type^mb_type)&(MB_FORWARD|MB_BACKWARD))==0
            && (!(mb_type&MB_FORWARD) ||
                (PMV[0][0][0]==vid_stream->mbinfo[k].MV[0][0][0] &&
                 PMV[0][0][1]==vid_stream->mbinfo[k].MV[0][0][1] &&
                 vid_stream->mbinfo[k].mv_field_sel[0][0]==(vid_stream->picture.pict_struct==BOTTOM_FIELD)))
            && (!(mb_type&MB_BACKWARD) ||
                (PMV[0][1][0]==vid_stream->mbinfo[k].MV[0][1][0] &&
                 PMV[0][1][1]==vid_stream->mbinfo[k].MV[0][1][1] &&
                 vid_stream->mbinfo[k].mv_field_sel[0][1]==(vid_stream->picture.pict_struct==BOTTOM_FIELD))))
        {
          /* conditions for skipping in B field pictures:
           * - must be field predicted
           * - must be the same prediction type (forward/backward/interp.)
           *   as previous macroblock
           * - relevant vectors (forward/backward/both) have to be the same
           *   as in previous macroblock
           * - relevant motion_vertical_field_selects have to be of same
           *   parity as current field
           */

          vid_stream->mbinfo[k].mb_type = mb_type;
          vid_stream->mbinfo[k].skipped = 1;
          MBAinc++;
          kold++;
          continue;
        }
      }

      /* macroblock cannot be skipped */
      vid_stream->mbinfo[k].skipped = 0;

      /* there's no VLC for 'No MC, Not Coded':
       * we have to transmit (0,0) motion vectors
       */
      if (vid_stream->picture.pict_type==P_TYPE && !cbp && !(mb_type&MB_FORWARD))
        mb_type|= MB_FORWARD;

      putaddrinc(vid_stream,MBAinc); /* macroblock_address_increment */
      MBAinc = 1;

      putmbtype(vid_stream,vid_stream->picture.pict_type,mb_type);

      if (mb_type & (MB_FORWARD|MB_BACKWARD) && !vid_stream->picture.frame_pred_dct)
        gst_putbits(&vid_stream->pb,vid_stream->mbinfo[k].motion_type,2);

      if (vid_stream->picture.pict_struct==FRAME_PICTURE && cbp && !vid_stream->picture.frame_pred_dct)
        gst_putbits(&vid_stream->pb,vid_stream->mbinfo[k].dct_type,1);

      if (mb_type & MB_QUANT)
      {
        gst_putbits(&vid_stream->pb,vid_stream->picture.q_scale_type ? map_non_linear_mquant[vid_stream->mbinfo[k].mquant]
                             : vid_stream->mbinfo[k].mquant>>1,5);
        prev_mquant = vid_stream->mbinfo[k].mquant;
      }

      if (mb_type & MB_FORWARD)
      {
        /* forward motion vectors, update predictors */
        putmvs(vid_stream,vid_stream->mbinfo[k].MV,PMV,vid_stream->mbinfo[k].mv_field_sel,vid_stream->mbinfo[k].dmvector,0,
          vid_stream->mbinfo[k].motion_type,vid_stream->picture.forw_hor_f_code,vid_stream->picture.forw_vert_f_code);
      }

      if (mb_type & MB_BACKWARD)
      {
        /* backward motion vectors, update predictors */
        putmvs(vid_stream,vid_stream->mbinfo[k].MV,PMV,vid_stream->mbinfo[k].mv_field_sel,vid_stream->mbinfo[k].dmvector,1,
          vid_stream->mbinfo[k].motion_type,vid_stream->picture.back_hor_f_code,vid_stream->picture.back_vert_f_code);
      }

      if (mb_type & MB_PATTERN)
      {
        putcbp(vid_stream,(cbp >> (vid_stream->seq.block_count-6)) & 63);
        if (vid_stream->seq.chroma_format!=CHROMA420)
          gst_putbits(&vid_stream->pb,cbp,vid_stream->seq.block_count-6);
      }

      for (comp=0; comp<vid_stream->seq.block_count; comp++)
      {
        /* block loop */
        if (cbp & (1<<(vid_stream->seq.block_count-1-comp)))
        {
          if (mb_type & MB_INTRA)
          {
            cc = (comp<4) ? 0 : (comp&1)+1;
            putintrablk(vid_stream, vid_stream->blocks[k*vid_stream->seq.block_count+comp],cc);
          }
          else
            putnonintrablk(vid_stream, vid_stream->blocks[k*vid_stream->seq.block_count+comp]);
        }
      }

      /* reset predictors */
      if (!(mb_type & MB_INTRA))
        for (cc=0; cc<3; cc++)
          vid_stream->dc_dct_pred[cc] = 0;

      if (mb_type & MB_INTRA || (vid_stream->picture.pict_type==P_TYPE && !(mb_type & MB_FORWARD)))
      {
        PMV[0][0][0]=PMV[0][0][1]=PMV[1][0][0]=PMV[1][0][1]=0;
        PMV[0][1][0]=PMV[0][1][1]=PMV[1][1][0]=PMV[1][1][1]=0;
      }

      vid_stream->mbinfo[k].mb_type = mb_type;
      kold++;
    }
  }

  if (real) {
    rc_update_pict(vid_stream);
    vbv_end_of_picture(vid_stream);
  }
}


/* output motion vectors (6.2.5.2, 6.3.16.2)
 *
 * this routine also updates the predictions for motion vectors (PMV)
 */
 
static void putmvs(vid_stream,MV,PMV,mv_field_sel,dmvector,s,motion_type,
  hor_f_code,vert_f_code)
mpeg2enc_vid_stream *vid_stream;
int MV[2][2][2],PMV[2][2][2];
int mv_field_sel[2][2];
int dmvector[2];
int s,motion_type,hor_f_code,vert_f_code;
{
  if (vid_stream->picture.pict_struct==FRAME_PICTURE)
  {
    if (motion_type==MC_FRAME)
    {
      /* frame prediction */
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(vid_stream,MV[0][s][1]-PMV[0][s][1],vert_f_code);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
    else if (motion_type==MC_FIELD)
    {
      /* field prediction */
      gst_putbits(&vid_stream->pb,mv_field_sel[0][s],1);
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(vid_stream,(MV[0][s][1]>>1)-(PMV[0][s][1]>>1),vert_f_code);
      gst_putbits(&vid_stream->pb,mv_field_sel[1][s],1);
      putmv(vid_stream,MV[1][s][0]-PMV[1][s][0],hor_f_code);
      putmv(vid_stream,(MV[1][s][1]>>1)-(PMV[1][s][1]>>1),vert_f_code);
      PMV[0][s][0]=MV[0][s][0];
      PMV[0][s][1]=MV[0][s][1];
      PMV[1][s][0]=MV[1][s][0];
      PMV[1][s][1]=MV[1][s][1];
    }
    else
    {
      /* dual prime prediction */
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putdmv(vid_stream,dmvector[0]);
      putmv(vid_stream,(MV[0][s][1]>>1)-(PMV[0][s][1]>>1),vert_f_code);
      putdmv(vid_stream,dmvector[1]);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
  }
  else
  {
    /* field picture */
    if (motion_type==MC_FIELD)
    {
      /* field prediction */
      gst_putbits(&vid_stream->pb,mv_field_sel[0][s],1);
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(vid_stream,MV[0][s][1]-PMV[0][s][1],vert_f_code);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
    else if (motion_type==MC_16X8)
    {
      /* 16x8 prediction */
      gst_putbits(&vid_stream->pb,mv_field_sel[0][s],1);
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putmv(vid_stream,MV[0][s][1]-PMV[0][s][1],vert_f_code);
      gst_putbits(&vid_stream->pb,mv_field_sel[1][s],1);
      putmv(vid_stream,MV[1][s][0]-PMV[1][s][0],hor_f_code);
      putmv(vid_stream,MV[1][s][1]-PMV[1][s][1],vert_f_code);
      PMV[0][s][0]=MV[0][s][0];
      PMV[0][s][1]=MV[0][s][1];
      PMV[1][s][0]=MV[1][s][0];
      PMV[1][s][1]=MV[1][s][1];
    }
    else
    {
      /* dual prime prediction */
      putmv(vid_stream,MV[0][s][0]-PMV[0][s][0],hor_f_code);
      putdmv(vid_stream,dmvector[0]);
      putmv(vid_stream,MV[0][s][1]-PMV[0][s][1],vert_f_code);
      putdmv(vid_stream,dmvector[1]);
      PMV[0][s][0]=PMV[1][s][0]=MV[0][s][0];
      PMV[0][s][1]=PMV[1][s][1]=MV[0][s][1];
    }
  }
}
