/* putseq.c, sequence level routines                                        */

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
#include <string.h>
#include "mpeg2enc.h"

void putseq(vid_stream, i)
mpeg2enc_vid_stream *vid_stream;
int i; /* frame number */
{
  /* this routine assumes (N % M) == 0 */
  int j, k, f, f0, n, np, nb, sxf=0, syf=0, sxb=0, syb=0, wanted;
  int ipflag;
  unsigned char *neworg[3], *newref[3];
  static char ipb[5] = {' ','I','P','B','D'};

  if (i == 0) {
    rc_init_seq(vid_stream); /* initialize rate control */

    /* sequence header, sequence extension and sequence display extension */
    putseqhdr(vid_stream);
    if (!vid_stream->mpeg1)
    {
      putseqext(vid_stream);
      putseqdispext(vid_stream);
    }

    /* optionally output some text data (description, copyright or whatever) */
    if (strlen(vid_stream->id_string) > 1)
      putuserdata(vid_stream, vid_stream->id_string);
  }

  /* loop through all frames in encoding/decoding order */
/*  for (i=0; i<vid_stream->nframes; i++) */
/*  {					  */
    if (!vid_stream->quiet)
    {
      printf("Encoding frame %d ",i);
    }

    /* f0: lowest frame number in current GOP
     *
     * first GOP contains N-(M-1) frames,
     * all other GOPs contain N frames
     */
    f0 = vid_stream->N*((i+(vid_stream->M-1))/vid_stream->N) - (vid_stream->M-1);

    if (f0<0)
      f0=0;

    if (i==0 || (i-1)%vid_stream->M==0)
    {
      /* I or P frame */
      for (j=0; j<3; j++)
      {
        /* shuffle reference frames */
        neworg[j] = vid_stream->oldorgframe[j];
        newref[j] = vid_stream->oldrefframe[j];
        vid_stream->oldorgframe[j] = vid_stream->neworgframe[j];
        vid_stream->oldrefframe[j] = vid_stream->newrefframe[j];
        vid_stream->neworgframe[j] = neworg[j];
        vid_stream->newrefframe[j] = newref[j];
      }

      /* f: frame number in display order */
      f = (i==0) ? 0 : i+vid_stream->M-1;
      if (f>=vid_stream->nframes)
        f = vid_stream->nframes - 1;

      if (i==f0) /* first displayed frame in GOP is I */
      {
        /* I frame */
        vid_stream->picture.pict_type = I_TYPE;
        vid_stream->picture.forw_hor_f_code = vid_stream->picture.forw_vert_f_code = 15;
        vid_stream->picture.back_hor_f_code = vid_stream->picture.back_vert_f_code = 15;

        /* n: number of frames in current GOP
         *
         * first GOP contains (M-1) less (B) frames
         */
        n = (i==0) ? vid_stream->N-(vid_stream->M-1) : vid_stream->N;

        /* last GOP may contain less frames */
        if (n > vid_stream->nframes-f0)
          n = vid_stream->nframes-f0;

        /* number of P frames */
        if (i==0)
          np = (n + 2*(vid_stream->M-1))/vid_stream->M - 1; /* first GOP */
        else
          np = (n + (vid_stream->M-1))/vid_stream->M - 1;

        /* number of B frames */
        nb = n - np - 1;

        rc_init_GOP(vid_stream,np,nb);

        putgophdr(vid_stream,f0,i==0); /* set closed_GOP in first GOP only */
      }
      else
      {
        /* P frame */
        vid_stream->picture.pict_type = P_TYPE;
        vid_stream->picture.forw_hor_f_code = vid_stream->motion_data[0].forw_hor_f_code;
        vid_stream->picture.forw_vert_f_code = vid_stream->motion_data[0].forw_vert_f_code;
        vid_stream->picture.back_hor_f_code = vid_stream->picture.back_vert_f_code = 15;
        sxf = vid_stream->motion_data[0].sxf;
        syf = vid_stream->motion_data[0].syf;
      }
    }
    else
    {
      /* B frame */
      for (j=0; j<3; j++)
      {
        neworg[j] = vid_stream->auxorgframe[j];
        newref[j] = vid_stream->auxframe[j];
      }

      /* f: frame number in display order */
      f = i - 1;
      vid_stream->picture.pict_type = B_TYPE;
      n = (i-2)%vid_stream->M + 1; /* first B: n=1, second B: n=2, ... */
      vid_stream->picture.forw_hor_f_code = vid_stream->motion_data[n].forw_hor_f_code;
      vid_stream->picture.forw_vert_f_code = vid_stream->motion_data[n].forw_vert_f_code;
      vid_stream->picture.back_hor_f_code = vid_stream->motion_data[n].back_hor_f_code;
      vid_stream->picture.back_vert_f_code = vid_stream->motion_data[n].back_vert_f_code;
      sxf = vid_stream->motion_data[n].sxf;
      syf = vid_stream->motion_data[n].syf;
      sxb = vid_stream->motion_data[n].sxb;
      syb = vid_stream->motion_data[n].syb;
    }

    vid_stream->picture.temp_ref = f - f0;
    vid_stream->picture.frame_pred_dct = vid_stream->picture.frame_pred_dct_tab[vid_stream->picture.pict_type-1];
    vid_stream->picture.q_scale_type = vid_stream->picture.qscale_tab[vid_stream->picture.pict_type-1];
    vid_stream->picture.intravlc = vid_stream->picture.intravlc_tab[vid_stream->picture.pict_type-1];
    vid_stream->picture.altscan = vid_stream->picture.altscan_tab[vid_stream->picture.pict_type-1];

    if (!vid_stream->quiet) {
      printf("\nFrame %d (#%d in display order):\n",i,f);
      printf(" picture_type=%c\n",ipb[vid_stream->picture.pict_type]);
      printf(" temporal_reference=%d\n",vid_stream->picture.temp_ref);
      printf(" frame_pred_frame_dct=%d\n",vid_stream->picture.frame_pred_dct);
      printf(" q_scale_type=%d\n",vid_stream->picture.q_scale_type);
      printf(" intra_vlc_format=%d\n",vid_stream->picture.intravlc);
      printf(" alternate_scan=%d\n",vid_stream->picture.altscan);

      if (vid_stream->picture.pict_type!=I_TYPE)
      {
       fprintf(stdout," forward search window: %d...%d / %d...%d\n",
          -sxf,sxf,-syf,syf);
        fprintf(stdout," forward vector range: %d...%d.5 / %d...%d.5\n",
          -(4<<vid_stream->picture.forw_hor_f_code),(4<<vid_stream->picture.forw_hor_f_code)-1,
          -(4<<vid_stream->picture.forw_vert_f_code),(4<<vid_stream->picture.forw_vert_f_code)-1);
      }

      if (vid_stream->picture.pict_type==B_TYPE)
      {
        fprintf(stdout," backward search window: %d...%d / %d...%d\n",
          -sxb,sxb,-syb,syb);
        fprintf(stdout," backward vector range: %d...%d.5 / %d...%d.5\n",
          -(4<<vid_stream->picture.back_hor_f_code),(4<<vid_stream->picture.back_hor_f_code)-1,
          -(4<<vid_stream->picture.back_vert_f_code),(4<<vid_stream->picture.back_vert_f_code)-1);
      }
    }

    if (i==0) wanted = 0;
    else wanted = (i+1)%vid_stream->M;

    memcpy(neworg[0], vid_stream->frame_buffer[wanted], vid_stream->seq.width*vid_stream->seq.height);
    memcpy(neworg[1], vid_stream->frame_buffer[wanted]+vid_stream->seq.width*vid_stream->seq.height,
    					(vid_stream->seq.width*vid_stream->seq.height)/4);
    memcpy(neworg[2], vid_stream->frame_buffer[wanted]+vid_stream->seq.width*vid_stream->seq.height+(vid_stream->seq.width*vid_stream->seq.height)/4,
    					(vid_stream->seq.width*vid_stream->seq.height)/4);

    if (vid_stream->fieldpic)
    {
      if (!vid_stream->quiet)
      {
        fprintf(stderr,"\nfirst field  (%s) ",vid_stream->picture.topfirst ? "top" : "bot");
        fflush(stderr);
      }

      vid_stream->picture.pict_struct = vid_stream->picture.topfirst ? TOP_FIELD : BOTTOM_FIELD;

      motion_estimation(vid_stream,vid_stream->oldorgframe[0],vid_stream->neworgframe[0],
                        vid_stream->oldrefframe[0],vid_stream->newrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,vid_stream->mbinfo,0,0);

      predict(vid_stream, vid_stream->oldrefframe,vid_stream->newrefframe,vid_stream->predframe,0,vid_stream->mbinfo);
      dct_type_estimation(vid_stream, vid_stream->predframe[0],neworg[0],vid_stream->mbinfo);
      transform(vid_stream, vid_stream->predframe,neworg,vid_stream->mbinfo,vid_stream->blocks);

      putpict(vid_stream,neworg[0]);

      for (k=0; k<vid_stream->seq.mb_height2*vid_stream->seq.mb_width; k++)
      {
        if (vid_stream->mbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<vid_stream->seq.block_count; j++)
            iquant_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                         vid_stream->picture.dc_prec,vid_stream->intra_q,vid_stream->mbinfo[k].mquant);
        else
          for (j=0;j<vid_stream->seq.block_count;j++)
            iquant_non_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                             vid_stream->inter_q,vid_stream->mbinfo[k].mquant);
      }

      itransform(vid_stream, vid_stream->predframe,newref,vid_stream->mbinfo,vid_stream->blocks);
      calcSNR(vid_stream,neworg,newref);
      if (!vid_stream->quiet) {
        stats(vid_stream);
      }

      if (!vid_stream->quiet)
      {
        fprintf(stderr,"second field (%s) ",vid_stream->picture.topfirst ? "bot" : "top");
        fflush(stderr);
      }

      vid_stream->picture.pict_struct = vid_stream->picture.topfirst ? BOTTOM_FIELD : TOP_FIELD;

      ipflag = (vid_stream->picture.pict_type==I_TYPE);
      if (ipflag)
      {
        /* first field = I, second field = P */
        vid_stream->picture.pict_type = P_TYPE;
        vid_stream->picture.forw_hor_f_code = vid_stream->motion_data[0].forw_hor_f_code;
        vid_stream->picture.forw_vert_f_code = vid_stream->motion_data[0].forw_vert_f_code;
        vid_stream->picture.back_hor_f_code = vid_stream->picture.back_vert_f_code = 15;
        sxf = vid_stream->motion_data[0].sxf;
        syf = vid_stream->motion_data[0].syf;
      }

      motion_estimation(vid_stream, vid_stream->oldorgframe[0],vid_stream->neworgframe[0],
                        vid_stream->oldrefframe[0],vid_stream->newrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,vid_stream->mbinfo,1,ipflag);

      predict(vid_stream, vid_stream->oldrefframe,vid_stream->newrefframe,vid_stream->predframe,1,vid_stream->mbinfo);
      dct_type_estimation(vid_stream, vid_stream->predframe[0],neworg[0],vid_stream->mbinfo);
      transform(vid_stream, vid_stream->predframe,neworg,vid_stream->mbinfo,vid_stream->blocks);

      putpict(vid_stream, neworg[0]);

      for (k=0; k<vid_stream->seq.mb_height2*vid_stream->seq.mb_width; k++)
      {
        if (vid_stream->mbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<vid_stream->seq.block_count; j++)
            iquant_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                         vid_stream->picture.dc_prec,vid_stream->intra_q,vid_stream->mbinfo[k].mquant);
        else
          for (j=0;j<vid_stream->seq.block_count;j++)
            iquant_non_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                             vid_stream->inter_q,vid_stream->mbinfo[k].mquant);
      }

      itransform(vid_stream, vid_stream->predframe,newref,vid_stream->mbinfo,vid_stream->blocks);
      calcSNR(vid_stream,neworg,newref);
      if (!vid_stream->quiet) {
        stats(vid_stream);
      }
    }
    else
    {
      vid_stream->picture.pict_struct = FRAME_PICTURE;

      /* do motion_estimation
       *
       * uses source frames (...orgframe) for full pel search
       * and reconstructed frames (...refframe) for half pel search
       */

      motion_estimation(vid_stream, vid_stream->oldorgframe[0],vid_stream->neworgframe[0],
                        vid_stream->oldrefframe[0],vid_stream->newrefframe[0],
                        neworg[0],newref[0],
                        sxf,syf,sxb,syb,vid_stream->mbinfo,0,0);

      predict(vid_stream, vid_stream->oldrefframe,vid_stream->newrefframe,vid_stream->predframe,0,vid_stream->mbinfo);
      dct_type_estimation(vid_stream, vid_stream->predframe[0],neworg[0],vid_stream->mbinfo);
      transform(vid_stream, vid_stream->predframe,neworg,vid_stream->mbinfo,vid_stream->blocks);

      putpict(vid_stream, neworg[0]);

      for (k=0; k<vid_stream->seq.mb_height*vid_stream->seq.mb_width; k++)
      {
        if (vid_stream->mbinfo[k].mb_type & MB_INTRA)
          for (j=0; j<vid_stream->seq.block_count; j++)
            iquant_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                         vid_stream->picture.dc_prec,vid_stream->intra_q,vid_stream->mbinfo[k].mquant);
        else
          for (j=0;j<vid_stream->seq.block_count;j++)
            iquant_non_intra(vid_stream,vid_stream->blocks[k*vid_stream->seq.block_count+j],vid_stream->blocks[k*vid_stream->seq.block_count+j],
                             vid_stream->inter_q,vid_stream->mbinfo[k].mquant);
      }

      itransform(vid_stream, vid_stream->predframe,newref,vid_stream->mbinfo,vid_stream->blocks);
      calcSNR(vid_stream,neworg,newref);
      if (!vid_stream->quiet) {
        stats(vid_stream);
      }
    }

    gst_putbits_align(&vid_stream->pb);

    /*sprintf(name,vid_stream->tplref,f+vid_stream->frame0);*/
    /*writeframe(name,newref);				    */

/*  }		*/

 /* putseqend(); */
}
