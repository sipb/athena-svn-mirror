/* mpeg2enc.c, main() and parameter file reading                            */

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

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/* private prototypes */
static void init _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
static void setparameters _ANSI_ARGS_ ((mpeg2enc_vid_stream *vid_stream));
static void readquantmat(mpeg2enc_vid_stream *vid_stream);


mpeg2enc_vid_stream *mpeg2enc_new_encoder() 
{
  mpeg2enc_vid_stream *new;

  new = (mpeg2enc_vid_stream *) malloc (sizeof(mpeg2enc_vid_stream));

  new->frame0 = 0;
  new->tc0 = 0;
  new->N = 15;
  new->M = 3;
  new->P = 0;
  new->mpeg1 = 1;
  new->inited = 0;
  new->nframes = 10000;
  new->framenum = 0;
  new->fieldpic = 0;
  new->quiet = 1;
  new->iqname[0] = '-';
  new->niqname[0] = '-';
  new->seq.horizontal_size = -1;
  new->seq.vertical_size = -1;
  new->seq.aspectratio = 9;
  new->seq.frame_rate_code = 3;
  /*new->seq.bit_rate = 300000.0;*/
  new->seq.bit_rate = 1150000.0;
  /*new->seq.bit_rate = 5000000.0;*/
  /*new->seq.bit_rate = 3000000.0;*/
  new->seq.vbv_buffer_size = 20;
  new->seq.low_delay = 1;
  new->seq.constrparms = 0;
  new->seq.profile = 1;
  new->seq.level = 4;
  new->seq.prog_seq = 1;
  new->seq.chroma_format = CHROMA420;
  new->seq.video_format = 0;
  new->seq.color_primaries = 1;
  new->seq.transfer_characteristics = 5;
  new->seq.matrix_coefficients = 7;
  new->seq.display_horizontal_size = -1;
  new->seq.display_vertical_size = -1;
  new->picture.dc_prec = 0;
  new->picture.topfirst = 0;
  new->picture.frame_pred_dct_tab[0] = 1;
  new->picture.frame_pred_dct_tab[1] = 1;
  new->picture.frame_pred_dct_tab[2] = 1;
  new->picture.conceal_tab[0] = 0;
  new->picture.conceal_tab[1] = 0;
  new->picture.conceal_tab[2] = 0;
  new->picture.qscale_tab[0] = 0;
  new->picture.qscale_tab[1] = 0;
  new->picture.qscale_tab[2] = 0;
  new->picture.q_scale_type = 0;
  new->picture.intravlc_tab[0] = 0;
  new->picture.intravlc_tab[1] = 0;
  new->picture.intravlc_tab[2] = 0;
  new->picture.altscan_tab[0] = 0;
  new->picture.altscan_tab[1] = 0;
  new->picture.altscan_tab[2] = 0;
  new->picture.repeatfirst = 0;
  new->picture.prog_frame = 1;
  new->idct = gst_idct_new(GST_IDCT_FLOAT);

  return new;
}

int mpeg2enc_new_picture (mpeg2enc_vid_stream *vid_stream, char *inbuf, int size, int encoder_state) 
{
  int i;
  if (!vid_stream->inited) setparameters(vid_stream);

  if (vid_stream->framenum == 0) {
    /*convertRGBtoYUV(vid_stream, inbuf, vid_stream->frame_buffer[0]);*/
    memcpy(vid_stream->frame_buffer[0], inbuf, MIN(vid_stream->frame_buffer_size, size));
    gst_putbits_new_empty_buffer(&vid_stream->pb, 1000000);
    printf("mpeg2enc: encoding %d", vid_stream->framenum);
    putseq(vid_stream, vid_stream->framenum);
    vid_stream->framenum++;
    return NEW_DATA;
  }
  else {
    /*convertRGBtoYUV(vid_stream, inbuf, vid_stream->frame_buffer[(vid_stream->framenum-1)%vid_stream->M]);*/
    memcpy(vid_stream->frame_buffer[(vid_stream->framenum-1)%vid_stream->M], inbuf, MIN(vid_stream->frame_buffer_size, size));

    if (vid_stream->framenum%vid_stream->M == 0) {
      gst_putbits_new_empty_buffer(&vid_stream->pb, 1000000);

      for (i=vid_stream->M-1; i>=0; i--) {
        printf("mpeg2enc: encoding %d", vid_stream->framenum-i);
        putseq(vid_stream, vid_stream->framenum-i);
      }
      vid_stream->framenum++;
      return NEW_DATA;
    }
  }
  vid_stream->framenum++;

  return 0;
}

static void init(vid_stream)
mpeg2enc_vid_stream *vid_stream;
{
  int i, size;
  static int block_count_tab[3] = {6,8,12};

  gst_putbits_init(&vid_stream->pb);
  init_fdct();
  init_quant(vid_stream);
  motion_estimation_init(vid_stream);
  transform_init(vid_stream);
  predict_init(vid_stream);

  /* round picture dimensions to nearest multiple of 16 or 32 */
  vid_stream->seq.mb_width = (vid_stream->seq.horizontal_size+15)/16;
  vid_stream->seq.mb_height = vid_stream->seq.prog_seq ? (vid_stream->seq.vertical_size+15)/16 : 2*((vid_stream->seq.vertical_size+31)/32);
  vid_stream->seq.mb_height2 = vid_stream->fieldpic ? vid_stream->seq.mb_height>>1 : vid_stream->seq.mb_height; /* for field pictures */
  vid_stream->seq.width = 16*vid_stream->seq.mb_width;
  vid_stream->seq.height = 16*vid_stream->seq.mb_height;
  vid_stream->MBPS = vid_stream->seq.mb_width * vid_stream->seq.mb_height;
  printf("mpeg2enc: width height %d %d\n", vid_stream->seq.width, vid_stream->seq.height); 

  vid_stream->seq.chrom_width = (vid_stream->seq.chroma_format==CHROMA444) ? vid_stream->seq.width : vid_stream->seq.width>>1;
  vid_stream->seq.chrom_height = (vid_stream->seq.chroma_format!=CHROMA420) ? vid_stream->seq.height : vid_stream->seq.height>>1;

  vid_stream->seq.height2 = vid_stream->fieldpic ? vid_stream->seq.height>>1 : vid_stream->seq.height;
  vid_stream->seq.width2 = vid_stream->fieldpic ? vid_stream->seq.width<<1 : vid_stream->seq.width;
  vid_stream->seq.chrom_width2 = vid_stream->fieldpic ? vid_stream->seq.chrom_width<<1 : vid_stream->seq.chrom_width;
  
  vid_stream->seq.block_count = block_count_tab[vid_stream->seq.chroma_format-1];

  /* clip table */
  if (!(vid_stream->clp = (unsigned char *)malloc(1024)))
    error("malloc failed\n");
  vid_stream->clp+= 384;
  for (i=-384; i<640; i++)
    vid_stream->clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

  for (i=0; i<3; i++)
  {
    size = (i==0) ? vid_stream->seq.width*vid_stream->seq.height : vid_stream->seq.chrom_width*vid_stream->seq.chrom_height;

    if (!(vid_stream->newrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->oldrefframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->auxframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->neworgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->oldorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->auxorgframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
    if (!(vid_stream->predframe[i] = (unsigned char *)malloc(size)))
      error("malloc failed\n");
  }

  vid_stream->mbinfo = (struct mbinfo *)malloc(vid_stream->seq.mb_width*vid_stream->seq.mb_height2*sizeof(struct mbinfo));
  vid_stream->mbinfo_temp = (struct mbinfo *)malloc(vid_stream->seq.mb_width*vid_stream->seq.mb_height2*sizeof(struct mbinfo));

  vid_stream->frame_buffer = (unsigned char **)malloc(vid_stream->M*sizeof(unsigned char *));

  vid_stream->frame_buffer_size = vid_stream->seq.width*vid_stream->seq.height+
		           vid_stream->seq.chrom_width*vid_stream->seq.chrom_height*4;

  for (i=0; i<vid_stream->M; i++) {
    vid_stream->frame_buffer[i] = (unsigned char *)malloc(vid_stream->frame_buffer_size);
  }

  if (!vid_stream->mbinfo)
    error("malloc failed\n");

  vid_stream->blocks =
    (short (*)[64])malloc(vid_stream->seq.mb_width*vid_stream->seq.mb_height2*vid_stream->seq.block_count*sizeof(short [64]));
  vid_stream->blocks_temp =
    (short (*)[64])malloc(vid_stream->seq.mb_width*vid_stream->seq.mb_height2*vid_stream->seq.block_count*sizeof(short [64]));

  if (!vid_stream->blocks)
    error("malloc failed\n");

  vid_stream->inited = 1;
}

void error(text)
char *text;
{
  fprintf(stderr,text);
  putc('\n',stderr);
  exit(1);
}

static void setparameters(mpeg2enc_vid_stream *vid_stream)
{
  int i;
  static double ratetab[8]=
    {24000.0/1001.0,24.0,25.0,30000.0/1001.0,30.0,50.0,60000.0/1001.0,60.0};
  /*extern int r,Xi,Xb,Xp,d0i,d0p,d0b;*/ /* rate control */
  /*extern double avg_act; */ /* rate control */

  if (vid_stream->N<1)
    error("N must be positive");
  if (vid_stream->M<1)
    error("M must be positive");
  if (vid_stream->N%vid_stream->M != 0)
    error("N must be an integer multiple of M");

  vid_stream->motion_data = (struct motion_data *)malloc(vid_stream->M*sizeof(struct motion_data));
  if (!vid_stream->motion_data)
    error("malloc failed\n");

  i = 0;
	  
  /*
  vid_stream->motion_data[i].forw_hor_f_code = 1; 
  vid_stream->motion_data[i].forw_vert_f_code = 1;
  vid_stream->motion_data[i].sxf =7; 
  vid_stream->motion_data[i].syf =7;

  i++;
  vid_stream->motion_data[i].forw_hor_f_code = 1; 
  vid_stream->motion_data[i].forw_vert_f_code = 1;
  vid_stream->motion_data[i].sxf =7; 
  vid_stream->motion_data[i].syf =7;

  vid_stream->motion_data[i].back_hor_f_code = 1; 
  vid_stream->motion_data[i].back_vert_f_code = 1;
  vid_stream->motion_data[i].sxb =7; 
  vid_stream->motion_data[i].syb =7;

  i++;
  vid_stream->motion_data[i].forw_hor_f_code = 1; 
  vid_stream->motion_data[i].forw_vert_f_code = 1;
  vid_stream->motion_data[i].sxf =7; 
  vid_stream->motion_data[i].syf =7;

  vid_stream->motion_data[i].back_hor_f_code = 1; 
  vid_stream->motion_data[i].back_vert_f_code = 1;
  vid_stream->motion_data[i].sxb =7; 
  vid_stream->motion_data[i].syb =7;
  */
  vid_stream->motion_data[i].forw_hor_f_code = 3; 
  vid_stream->motion_data[i].forw_vert_f_code = 3;
  vid_stream->motion_data[i].sxf =63; 
  vid_stream->motion_data[i].syf =63;

  i++;
  vid_stream->motion_data[i].forw_hor_f_code = 2; 
  vid_stream->motion_data[i].forw_vert_f_code = 2;
  vid_stream->motion_data[i].sxf =31; 
  vid_stream->motion_data[i].syf =31;

  vid_stream->motion_data[i].back_hor_f_code = 2; 
  vid_stream->motion_data[i].back_vert_f_code = 2;
  vid_stream->motion_data[i].sxb =31; 
  vid_stream->motion_data[i].syb =31;

  i++;
  vid_stream->motion_data[i].forw_hor_f_code = 2; 
  vid_stream->motion_data[i].forw_vert_f_code = 2;
  vid_stream->motion_data[i].sxf =31; 
  vid_stream->motion_data[i].syf =31;

  vid_stream->motion_data[i].back_hor_f_code = 2; 
  vid_stream->motion_data[i].back_vert_f_code = 2;
  vid_stream->motion_data[i].sxb =31; 
  vid_stream->motion_data[i].syb =31;

  /* make sure MPEG specific parameters are valid */
  range_checks(vid_stream);

  vid_stream->seq.frame_rate = ratetab[vid_stream->seq.frame_rate_code-1];

  if (!vid_stream->mpeg1)
  {
    profile_and_level_checks(vid_stream);
  }
  else
  {
    /* MPEG-1 */
    if (vid_stream->seq.constrparms)
    {
      if (vid_stream->seq.horizontal_size>768
          || vid_stream->seq.vertical_size>576
          || ((vid_stream->seq.horizontal_size+15)/16)*((vid_stream->seq.vertical_size+15)/16)>396
          || ((vid_stream->seq.horizontal_size+15)/16)*((vid_stream->seq.vertical_size+15)/16)*vid_stream->seq.frame_rate>396*25.0
          || vid_stream->seq.frame_rate>30.0)
      {
        if (!vid_stream->quiet)
          fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
        vid_stream->seq.constrparms = 0;
      }
    }

    if (vid_stream->seq.constrparms)
    {
      for (i=0; i<vid_stream->M; i++)
      {
        if (vid_stream->motion_data[i].forw_hor_f_code>4)
        {
          if (!vid_stream->quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          vid_stream->seq.constrparms = 0;
          break;
        }

        if (vid_stream->motion_data[i].forw_vert_f_code>4)
        {
          if (!vid_stream->quiet)
            fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
          vid_stream->seq.constrparms = 0;
          break;
        }

        if (i!=0)
        {
          if (vid_stream->motion_data[i].back_hor_f_code>4)
          {
            if (!vid_stream->quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            vid_stream->seq.constrparms = 0;
            break;
          }

          if (vid_stream->motion_data[i].back_vert_f_code>4)
          {
            if (!vid_stream->quiet)
              fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
            vid_stream->seq.constrparms = 0;
            break;
          }
        }
      }
    }
  }

  /* relational checks */

  if (vid_stream->mpeg1)
  {
    if (!vid_stream->seq.prog_seq)
    {
      if (!vid_stream->quiet)
        fprintf(stderr,"Warning: setting progressive_sequence = 1\n");
      vid_stream->seq.prog_seq = 1;
    }

    if (vid_stream->seq.chroma_format!=CHROMA420)
	 {
      if (!vid_stream->quiet)
        fprintf(stderr,"Warning: setting chroma_format = 1 (4:2:0)\n");
      vid_stream->seq.chroma_format = CHROMA420;
    }

    if (vid_stream->picture.dc_prec!=0)
    {
      if (!vid_stream->quiet)
        fprintf(stderr,"Warning: setting intra_dc_precision = 0\n");
      vid_stream->picture.dc_prec = 0;
    }

    for (i=0; i<3; i++)
      if (vid_stream->picture.qscale_tab[i])
      {
        if (!vid_stream->quiet)
          fprintf(stderr,"Warning: setting qscale_tab[%d] = 0\n",i);
        vid_stream->picture.qscale_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (vid_stream->picture.intravlc_tab[i])
      {
        if (!vid_stream->quiet)
          fprintf(stderr,"Warning: setting intravlc_tab[%d] = 0\n",i);
        vid_stream->picture.intravlc_tab[i] = 0;
      }

    for (i=0; i<3; i++)
      if (vid_stream->picture.altscan_tab[i])
      {
        if (!vid_stream->quiet)
          fprintf(stderr,"Warning: setting altscan_tab[%d] = 0\n",i);
        vid_stream->picture.altscan_tab[i] = 0;
      }
  }

  if (!vid_stream->mpeg1 && vid_stream->seq.constrparms)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
    vid_stream->seq.constrparms = 0;
  }

  if (vid_stream->seq.prog_seq && !vid_stream->picture.prog_frame)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"Warning: setting progressive_frame = 1\n");
    vid_stream->picture.prog_frame = 1;
  }

  if (vid_stream->picture.prog_frame && vid_stream->fieldpic)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"Warning: setting field_pictures = 0\n");
    vid_stream->fieldpic = 0;
  }

  if (!vid_stream->picture.prog_frame && vid_stream->picture.repeatfirst)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"Warning: setting repeat_first_field = 0\n");
    vid_stream->picture.repeatfirst = 0;
  }

  if (vid_stream->picture.prog_frame)
  {
    for (i=0; i<3; i++)
      if (!vid_stream->picture.frame_pred_dct_tab[i])
      {
        if (!vid_stream->quiet)
          fprintf(stderr,"Warning: setting frame_pred_frame_dct[%d] = 1\n",i);
        vid_stream->picture.frame_pred_dct_tab[i] = 1;
      }
  }

  if (vid_stream->seq.prog_seq && !vid_stream->picture.repeatfirst && vid_stream->picture.topfirst)
  {
    if (!vid_stream->quiet)
      fprintf(stderr,"Warning: setting top_field_first = 0\n");
    vid_stream->picture.topfirst = 0;
  }

  /* search windows */
  for (i=0; i<vid_stream->M; i++)
  {
    if (vid_stream->motion_data[i].sxf > (4<<vid_stream->motion_data[i].forw_hor_f_code)-1)
    {
      if (!vid_stream->quiet)
        fprintf(stderr,
          "Warning: reducing forward horizontal search width to %d\n",
          (4<<vid_stream->motion_data[i].forw_hor_f_code)-1);
      vid_stream->motion_data[i].sxf = (4<<vid_stream->motion_data[i].forw_hor_f_code)-1;
    }

    if (vid_stream->motion_data[i].syf > (4<<vid_stream->motion_data[i].forw_vert_f_code)-1)
    {
      if (!vid_stream->quiet)
        fprintf(stderr,
          "Warning: reducing forward vertical search width to %d\n",
          (4<<vid_stream->motion_data[i].forw_vert_f_code)-1);
      vid_stream->motion_data[i].syf = (4<<vid_stream->motion_data[i].forw_vert_f_code)-1;
    }

    if (i!=0)
    {
      if (vid_stream->motion_data[i].sxb > (4<<vid_stream->motion_data[i].back_hor_f_code)-1)
      {
        if (!vid_stream->quiet)
          fprintf(stderr,
            "Warning: reducing backward horizontal search width to %d\n",
            (4<<vid_stream->motion_data[i].back_hor_f_code)-1);
        vid_stream->motion_data[i].sxb = (4<<vid_stream->motion_data[i].back_hor_f_code)-1;
      }

      if (vid_stream->motion_data[i].syb > (4<<vid_stream->motion_data[i].back_vert_f_code)-1)
      {
        if (!vid_stream->quiet)
          fprintf(stderr,
            "Warning: reducing backward vertical search width to %d\n",
            (4<<vid_stream->motion_data[i].back_vert_f_code)-1);
        vid_stream->motion_data[i].syb = (4<<vid_stream->motion_data[i].back_vert_f_code)-1;
      }
    }
  }

  init(vid_stream);
  readquantmat(vid_stream);

}

static void readquantmat(mpeg2enc_vid_stream *vid_stream)
{
  int i,v;
  FILE *fd;

  if (vid_stream->iqname[0]=='-')
  {
    /* use default intra matrix */
    vid_stream->seq.load_iquant = 0;
    for (i=0; i<64; i++) {
      vid_stream->intra_q[i] = default_intra_quantizer_matrix[i];
      vid_stream->i_intra_q[i] = (int)(((double)(IQUANT_SCALE)) /
            (double)(default_intra_quantizer_matrix[i]));
    }
  }
  else
  {
    /* read customized intra matrix */
    vid_stream->seq.load_iquant = 1;
    if (!(fd = fopen(vid_stream->iqname,"r")))
    {
      printf("Couldn't open quant matrix file %s",vid_stream->iqname);
      exit(0);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      vid_stream->intra_q[i] = v;

    }

    fclose(fd);
  }

  if (vid_stream->niqname[0]=='-')
  {
    /* use default non-intra matrix */
    vid_stream->seq.load_niquant = 0;
    for (i=0; i<64; i++) {
      vid_stream->inter_q[i] = default_non_intra_quantizer_matrix[i];
      vid_stream->i_inter_q[i] = (int)(((double)(IQUANT_SCALE)) /
            (double)(vid_stream->inter_q[i]));
    }
  }
  else
  {
    /* read customized non-intra matrix */
    vid_stream->seq.load_niquant = 1;
    if (!(fd = fopen(vid_stream->niqname,"r")))
    {
      printf("Couldn't open quant matrix file %s",vid_stream->niqname);
      exit(0);
    }

    for (i=0; i<64; i++)
    {
      fscanf(fd,"%d",&v);
      if (v<1 || v>255)
        error("invalid value in quant matrix");
      vid_stream->inter_q[i] = v;
    }

    fclose(fd);
  }
}
