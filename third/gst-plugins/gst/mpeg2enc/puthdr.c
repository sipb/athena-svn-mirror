/* puthdr.c, generation of headers                                          */

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
#include "mpeg2enc.h"

/* private prototypes */
static int frametotc _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int frame));

/* generate sequence header (6.2.2.1, 6.3.3)
 *
 * matrix download not implemented
 */
void putseqhdr(mpeg2enc_vid_stream *vid_stream)
{
  int i;

  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,SEQ_START_CODE,32); /* sequence_header_code */
  gst_putbits(&vid_stream->pb,vid_stream->seq.horizontal_size,12); /* horizontal_size_value */
  gst_putbits(&vid_stream->pb,vid_stream->seq.vertical_size,12); /* vertical_size_value */
  gst_putbits(&vid_stream->pb,vid_stream->seq.aspectratio,4); /* aspect_ratio_information */
  gst_putbits(&vid_stream->pb,vid_stream->seq.frame_rate_code,4); /* frame_rate_code */
  gst_putbits(&vid_stream->pb,(int)ceil(vid_stream->seq.bit_rate/400.0),18); /* bit_rate_value */
  gst_putbits(&vid_stream->pb,1,1); /* marker_bit */
  gst_putbits(&vid_stream->pb,vid_stream->seq.vbv_buffer_size,10); /* vbv_buffer_size_value */
  gst_putbits(&vid_stream->pb,vid_stream->seq.constrparms,1); /* constrained_parameters_flag */

  gst_putbits(&vid_stream->pb,vid_stream->seq.load_iquant,1); /* load_intra_quantizer_matrix */
  if (vid_stream->seq.load_iquant)
    for (i=0; i<64; i++)  /* matrices are always downloaded in zig-zag order */
      gst_putbits(&vid_stream->pb,vid_stream->intra_q[zig_zag_scan[i]],8); /* intra_quantizer_matrix */

  gst_putbits(&vid_stream->pb,vid_stream->seq.load_niquant,1); /* load_non_intra_quantizer_matrix */
  if (vid_stream->seq.load_niquant)
    for (i=0; i<64; i++)
      gst_putbits(&vid_stream->pb,vid_stream->inter_q[zig_zag_scan[i]],8); /* non_intra_quantizer_matrix */
}

/* generate sequence extension (6.2.2.3, 6.3.5) header (MPEG-2 only) */
void putseqext(mpeg2enc_vid_stream *vid_stream)
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,EXT_START_CODE,32); /* extension_start_code */
  gst_putbits(&vid_stream->pb,SEQ_ID,4); /* extension_start_code_identifier */
  gst_putbits(&vid_stream->pb,(vid_stream->seq.profile<<4)|vid_stream->seq.level,8); /* profile_and_level_indication */
  gst_putbits(&vid_stream->pb,vid_stream->seq.prog_seq,1); /* progressive sequence */
  gst_putbits(&vid_stream->pb,vid_stream->seq.chroma_format,2); /* chroma_format */
  gst_putbits(&vid_stream->pb,vid_stream->seq.horizontal_size>>12,2); /* horizontal_size_extension */
  gst_putbits(&vid_stream->pb,vid_stream->seq.vertical_size>>12,2); /* vertical_size_extension */
  gst_putbits(&vid_stream->pb,((int)ceil(vid_stream->seq.bit_rate/400.0))>>18,12); /* bit_rate_extension */
  gst_putbits(&vid_stream->pb,1,1); /* marker_bit */
  gst_putbits(&vid_stream->pb,vid_stream->seq.vbv_buffer_size>>10,8); /* vbv_buffer_size_extension */
  gst_putbits(&vid_stream->pb,0,1); /* low_delay  -- currently not implemented */
  gst_putbits(&vid_stream->pb,0,2); /* frame_rate_extension_n */
  gst_putbits(&vid_stream->pb,0,5); /* frame_rate_extension_d */
}

/* generate sequence display extension (6.2.2.4, 6.3.6)
 *
 * content not yet user setable
 */
void putseqdispext(mpeg2enc_vid_stream *vid_stream)
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,EXT_START_CODE,32); /* extension_start_code */
  gst_putbits(&vid_stream->pb,DISP_ID,4); /* extension_start_code_identifier */
  gst_putbits(&vid_stream->pb,vid_stream->seq.video_format,3); /* video_format */
  gst_putbits(&vid_stream->pb,1,1); /* colour_description */
  gst_putbits(&vid_stream->pb,vid_stream->seq.color_primaries,8); /* colour_primaries */
  gst_putbits(&vid_stream->pb,vid_stream->seq.transfer_characteristics,8); /* transfer_characteristics */
  gst_putbits(&vid_stream->pb,vid_stream->seq.matrix_coefficients,8); /* matrix_coefficients */
  gst_putbits(&vid_stream->pb,vid_stream->seq.display_horizontal_size,14); /* display_horizontal_size */
  gst_putbits(&vid_stream->pb,1,1); /* marker_bit */
  gst_putbits(&vid_stream->pb,vid_stream->seq.display_vertical_size,14); /* display_vertical_size */
}

/* output a zero terminated string as user data (6.2.2.2.2, 6.3.4.1)
 *
 * string must not emulate start codes
 */
void putuserdata(vid_stream,userdata)
mpeg2enc_vid_stream *vid_stream;
char *userdata;
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,USER_START_CODE,32); /* user_data_start_code */
  while (*userdata)
    gst_putbits(&vid_stream->pb,*userdata++,8);
}

/* generate group of pictures header (6.2.2.6, 6.3.9)
 *
 * uses tc0 (timecode of first frame) and frame0 (number of first frame)
 */
void putgophdr(vid_stream,frame,closed_gop)
mpeg2enc_vid_stream *vid_stream;
int frame,closed_gop;
{
  int tc;

  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,GOP_START_CODE,32); /* group_start_code */
  tc = frametotc(vid_stream,vid_stream->tc0+frame);
  gst_putbits(&vid_stream->pb,tc,25); /* time_code */
  gst_putbits(&vid_stream->pb,closed_gop,1); /* closed_gop */
  gst_putbits(&vid_stream->pb,0,1); /* broken_link */
}

/* convert frame number to time_code
 *
 * drop_frame not implemented
 */
static int frametotc(vid_stream,frame)
mpeg2enc_vid_stream *vid_stream;
int frame;
{
  int fps, pict, sec, minute, hour, tc;

  fps = (int)(vid_stream->seq.frame_rate+0.5);
  pict = frame%fps;
  frame = (frame-pict)/fps;
  sec = frame%60;
  frame = (frame-sec)/60;
  minute = frame%60;
  frame = (frame-minute)/60;
  hour = frame%24;
  tc = (hour<<19) | (minute<<13) | (1<<12) | (sec<<6) | pict;

  return tc;
}

/* generate picture header (6.2.3, 6.3.10) */
void putpicthdr(mpeg2enc_vid_stream *vid_stream)
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,PICTURE_START_CODE,32); /* picture_start_code */
  calc_vbv_delay(vid_stream);
  gst_putbits(&vid_stream->pb,vid_stream->picture.temp_ref,10); /* temporal_reference */
  gst_putbits(&vid_stream->pb,vid_stream->picture.pict_type,3); /* picture_coding_type */
  gst_putbits(&vid_stream->pb,vid_stream->picture.vbv_delay,16); /* vbv_delay */

  if (vid_stream->picture.pict_type==P_TYPE || vid_stream->picture.pict_type==B_TYPE)
  {
    gst_putbits(&vid_stream->pb,0,1); /* full_pel_forward_vector */
    if (vid_stream->mpeg1)
      gst_putbits(&vid_stream->pb,vid_stream->picture.forw_hor_f_code,3);
    else
      gst_putbits(&vid_stream->pb,7,3); /* forward_f_code */
  }

  if (vid_stream->picture.pict_type==B_TYPE)
  {
    gst_putbits(&vid_stream->pb,0,1); /* full_pel_backward_vector */
    if (vid_stream->mpeg1)
      gst_putbits(&vid_stream->pb,vid_stream->picture.back_hor_f_code,3);
    else
      gst_putbits(&vid_stream->pb,7,3); /* backward_f_code */
  }

  gst_putbits(&vid_stream->pb,0,1); /* extra_bit_picture */
}

/* generate picture coding extension (6.2.3.1, 6.3.11)
 *
 * composite display information (v_axis etc.) not implemented
 */
void putpictcodext(mpeg2enc_vid_stream *vid_stream)
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,EXT_START_CODE,32); /* extension_start_code */
  gst_putbits(&vid_stream->pb,CODING_ID,4); /* extension_start_code_identifier */
  gst_putbits(&vid_stream->pb,vid_stream->picture.forw_hor_f_code,4); /* forward_horizontal_f_code */
  gst_putbits(&vid_stream->pb,vid_stream->picture.forw_vert_f_code,4); /* forward_vertical_f_code */
  gst_putbits(&vid_stream->pb,vid_stream->picture.back_hor_f_code,4); /* backward_horizontal_f_code */
  gst_putbits(&vid_stream->pb,vid_stream->picture.back_vert_f_code,4); /* backward_vertical_f_code */
  gst_putbits(&vid_stream->pb,vid_stream->picture.dc_prec,2); /* intra_dc_precision */
  gst_putbits(&vid_stream->pb,vid_stream->picture.pict_struct,2); /* picture_structure */
  gst_putbits(&vid_stream->pb,(vid_stream->picture.pict_struct==FRAME_PICTURE)?vid_stream->picture.topfirst:0,1); /* top_field_first */
  gst_putbits(&vid_stream->pb,vid_stream->picture.frame_pred_dct,1); /* frame_pred_frame_dct */
  gst_putbits(&vid_stream->pb,0,1); /* concealment_motion_vectors  -- currently not implemented */
  gst_putbits(&vid_stream->pb,vid_stream->picture.q_scale_type,1); /* q_scale_type */
  gst_putbits(&vid_stream->pb,vid_stream->picture.intravlc,1); /* intra_vlc_format */
  gst_putbits(&vid_stream->pb,vid_stream->picture.altscan,1); /* alternate_scan */
  gst_putbits(&vid_stream->pb,vid_stream->picture.repeatfirst,1); /* repeat_first_field */
  gst_putbits(&vid_stream->pb,vid_stream->picture.prog_frame,1); /* chroma_420_type */
  gst_putbits(&vid_stream->pb,vid_stream->picture.prog_frame,1); /* progressive_frame */
  gst_putbits(&vid_stream->pb,0,1); /* composite_display_flag */
}

/* generate sequence_end_code (6.2.2) */
void putseqend(mpeg2enc_vid_stream *vid_stream)
{
  gst_putbits_align(&vid_stream->pb);
  gst_putbits(&vid_stream->pb,SEQ_END_CODE,32);
}
