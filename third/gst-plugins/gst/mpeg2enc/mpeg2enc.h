/* mpg2enc.h, defines and types                                             */

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
#include "config.h"
#include <gst/putbits/putbits.h>
#include <gst/idct/idct.h>
#include <gst/gstcpu.h>

/*#undef HAVE_LIBMMX*/

#define PICTURE_START_CODE 0x100L
#define SLICE_MIN_START    0x101L
#define SLICE_MAX_START    0x1AFL
#define USER_START_CODE    0x1B2L
#define SEQ_START_CODE     0x1B3L
#define EXT_START_CODE     0x1B5L
#define SEQ_END_CODE       0x1B7L
#define GOP_START_CODE     0x1B8L
#define ISO_END_CODE       0x1B9L
#define PACK_START_CODE    0x1BAL
#define SYSTEM_START_CODE  0x1BBL

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

/* picture structure */
#define TOP_FIELD     1
#define BOTTOM_FIELD  2
#define FRAME_PICTURE 3

/* macroblock type */
#define MB_INTRA    1
#define MB_PATTERN  2
#define MB_BACKWARD 4
#define MB_FORWARD  8
#define MB_QUANT    16

/* motion_type */
#define MC_FIELD 1
#define MC_FRAME 2
#define MC_16X8  2
#define MC_DMV   3

/* mv_format */
#define MV_FIELD 0
#define MV_FRAME 1

/* chroma_format */
#define CHROMA420 1
#define CHROMA422 2
#define CHROMA444 3

/* extension start code IDs */

#define SEQ_ID       1
#define DISP_ID      2
#define QUANT_ID     3
#define SEQSCAL_ID   5
#define PANSCAN_ID   7
#define CODING_ID    8
#define SPATSCAL_ID  9
#define TEMPSCAL_ID 10

/* inputtype */
#define T_Y_U_V 0
#define T_YUV   1
#define T_PPM   2

/* encoder state */
#define NEW_DATA 1

/* macroblock information */
struct mbinfo {
  int mb_type; /* intra/forward/backward/interpolated */
  int motion_type; /* frame/field/16x8/dual_prime */
  int dct_type; /* field/frame DCT */
  int mquant; /* quantization parameter */
  int cbp; /* coded block pattern */
  int skipped; /* skipped macroblock */
  int MV[2][2][2]; /* motion vectors */
  int mv_field_sel[2][2]; /* motion vertical field select */
  int dmvector[2]; /* dual prime vectors */
  double act; /* activity measure */
  int var; /* for debugging */
  short *dctblocks;
};

/* motion data */
struct motion_data {
  int forw_hor_f_code,forw_vert_f_code; /* vector range */
  int sxf,syf; /* search range */
  int back_hor_f_code,back_vert_f_code;
  int sxb,syb;
};

#ifdef NON_ANSI_COMPILER
#define _ANSI_ARGS_(x) ()
#else
#define _ANSI_ARGS_(x) x
#endif

/* prototypes of global functions */
extern unsigned char zig_zag_scan[64];
extern unsigned char alternate_scan[64];
extern unsigned char default_intra_quantizer_matrix[64];
extern unsigned char default_non_intra_quantizer_matrix[64];
extern unsigned char non_linear_mquant_table[32];
extern unsigned char map_non_linear_mquant[113];


/* fdctref.c */
void init_fdct _ANSI_ARGS_((void));
void fdct _ANSI_ARGS_((short *block));

/* mpeg2enc.c */
void error _ANSI_ARGS_((char *text));

/* writepic.c */
void writeframe _ANSI_ARGS_((char *fname, unsigned char *frame[]));


typedef struct Sequence {
  /* sequence specific data (sequence header) */
  int horizontal_size, vertical_size; /* frame size (pels) */
  int width, height; /* encoded frame size (pels) multiples of 16 or 32 */
  int chrom_width,chrom_height,block_count;
  int mb_width, mb_height; /* frame size (macroblocks) */
  int width2, height2, mb_height2, chrom_width2; /* picture size */
  int aspectratio; /* aspect ratio information (pel or display) */
  int frame_rate_code; /* coded value of frame rate */
  double frame_rate; /* frames per second */
  double bit_rate; /* bits per second */
  int vbv_buffer_size; /* size of VBV buffer (* 16 kbit) */
  int constrparms; /* constrained parameters flag (MPEG-1 only) */
  int load_iquant, load_niquant; /* use non-default quant. matrices */
  int load_ciquant,load_cniquant;

  /* sequence specific data (sequence extension) */
  int profile, level; /* syntax / parameter constraints */
  int prog_seq; /* progressive sequence */
  int chroma_format;
  int low_delay; /* no B pictures, skipped pictures */


  /* sequence specific data (sequence display extension) */
  int video_format; /* component, PAL, NTSC, SECAM or MAC */
  int color_primaries; /* source primary chromaticity coordinates */
  int transfer_characteristics; /* opto-electronic transfer char. (gamma) */
  int matrix_coefficients; /* Eg,Eb,Er / Y,Cb,Cr matrix coefficients */
  int display_horizontal_size, display_vertical_size; /* display size */

} mpeg2enc_Sequence;


typedef struct Picture {
  /* picture specific data (picture header) */
  int temp_ref; /* temporal reference */
  int pict_type; /* picture coding type (I, P or B) */
  int vbv_delay; /* video buffering verifier delay (1/90000 seconds) */

  /* picture specific data (picture coding extension) */
  int forw_hor_f_code, forw_vert_f_code;
  int back_hor_f_code, back_vert_f_code; /* motion vector ranges */
  int dc_prec; /* DC coefficient precision for intra coded blocks */
  int pict_struct; /* picture structure (frame, top / bottom field) */
  int topfirst; /* display top field first */
  /* use only frame prediction and frame DCT (I,P,B,current) */
  int frame_pred_dct_tab[3], frame_pred_dct;
  int conceal_tab[3]; /* use concealment motion vectors (I,P,B) */
  int qscale_tab[3], q_scale_type; /* linear/non-linear quantizaton table */
  int intravlc_tab[3], intravlc; /* intra vlc format (I,P,B,current) */
  int altscan_tab[3], altscan; /* alternate scan (I,P,B,current) */
  int repeatfirst; /* repeat first field after second field */
  int prog_frame; /* progressive frame */

} mpeg2enc_Picture;

/* picture data arrays */
typedef struct vid_stream {
  /* reconstructed frames */
  unsigned char *newrefframe[3], *oldrefframe[3], *auxframe[3];
  /* original frames */
  unsigned char *neworgframe[3], *oldorgframe[3], *auxorgframe[3];
  /* prediction of current frame */
  unsigned char *predframe[3];
  unsigned long frame_buffer_size;
  unsigned char **frame_buffer;
  /* 8*8 block data */
  short (*blocks)[64];
  short (*blocks_temp)[64];
  /* intra / non_intra quantization matrices */
  unsigned short intra_q[64], inter_q[64];
  unsigned short i_intra_q[64], i_inter_q[64];
  unsigned short chrom_intra_q[64],chrom_inter_q[64];
  /* prediction values for DCT coefficient (0,0) */
  int dc_dct_pred[3];
  /* macroblock side information array */
  struct mbinfo *mbinfo;
  struct mbinfo *mbinfo_temp;
  /* motion estimation parameters */
  struct motion_data *motion_data;
  /* clipping (=saturation) table */
  unsigned char *clp;

  unsigned int MBPS; /* macroblocks per slice */

 /* name strings */
  char id_string[256], tplorg[256], tplref[256];
  char iqname[256], niqname[256];
  char statname[256];
  char errortext[256];

  FILE *outfile, *statfile; /* file descriptors */
  int inputtype; /* format of input frames */

  int quiet; /* suppress warnings */

  mpeg2enc_Sequence seq;
  mpeg2enc_Picture picture;

  /* coding model parameters */
  int N; /* number of frames in Group of Pictures */
  int M; /* distance between I/P frames */
  int P; /* intra slice refresh interval */
  int nframes; /* total number of frames to encode */
  int frame0, tc0; /* number and timecode of first frame */
  int mpeg1; /* ISO/IEC IS 11172-2 sequence */
  int fieldpic; /* use field pictures */

  gst_putbits_t pb;
  GstIDCT *idct;

  int inited;
  int framenum;

} mpeg2enc_vid_stream;

/* conform.c */
void range_checks _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void profile_and_level_checks _ANSI_ARGS_(());

/* motion.c */
void motion_estimation_init _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));

void motion_estimation _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *oldorg, unsigned char *neworg,
  unsigned char *oldref, unsigned char *newref, unsigned char *cur,
  unsigned char *curref, int sxf, int syf, int sxb, int syb,
  struct mbinfo *mbi, int secondfield, int ipflag));

/* predict.c */
void predict_init _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void predict _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *reff[], unsigned char *refb[],
  unsigned char *cur[3], int secondfield, struct mbinfo *mbi));

/* puthdr.c */
void putseqhdr _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void putseqext _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void putseqdispext _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void putuserdata _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, char *userdata));
void putgophdr _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, int frame, int closed_gop));
void putpicthdr _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void putpictcodext _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void putseqend _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));

/* putmpg.c */
void putintrablk _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,short *blk, int cc));
void putnonintrablk _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,short *blk));
void putmv _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, int dmv, int f_code));

/* putpic.c */
void putpict _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *frame));

/* putseq.c */
void putseq _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, int framenum));

/* putvlc.c */
void putDClum _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int val));
void putDCchrom _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int val));
void putACfirst _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int run, int val));
void putAC _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int run, int signed_level, int vlcformat));
void putaddrinc _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int addrinc));
void putmbtype _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int pict_type, int mb_type));
void putmotioncode _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int motion_code));
void putdmv _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int dmv));
void putcbp _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int cbp));

/* SCale factor for fast integer arithmetic routines */
/* Changed this and you *must* change the quantisation routines as they depend on its absolute
 *         value */
#define IQUANT_SCALE_POW2 16
#define IQUANT_SCALE (1<<IQUANT_SCALE_POW2)

#define COEFFSUM_SCALE (1<<16)

/* quantize.c */
void init_quant _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
int quant_intra _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,short *src, short *dst, int dc_prec,
  unsigned short *quant_mat, int mquant));
int quant_non_intra _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,short *src, short *dst,
  unsigned short *quant_mat, int mquant));
void iquant_intra _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, short *src, short *dst, int dc_prec,
  unsigned short *quant_mat, int mquant));
void iquant_non_intra _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, short *src, short *dst,
  unsigned short *quant_mat, int mquant));
extern int (*quant_weight_coeff_sum)( short *blk, unsigned short * i_quant_mat );


/* ratectl.c */
void rc_init_seq _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void rc_init_GOP _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,int np, int nb));
void rc_init_pict _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *frame));
void rc_update_pict _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
int rc_start_mb _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
int rc_calc_mquant _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, int j, int seq));
void vbv_end_of_picture _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void calc_vbv_delay _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));


/* stats.c */
void calcSNR _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *org[3], unsigned char *rec[3]));
void stats _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));

/* transfrm.c */
void transform_init _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream));
void transform _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *pred[], unsigned char *cur[],
  struct mbinfo *mbi, short blocks[][64]));
void itransform _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *pred[], unsigned char *cur[],
  struct mbinfo *mbi, short blocks[][64]));
void dct_type_estimation _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream,unsigned char *pred, unsigned char *cur,
  struct mbinfo *mbi));

mpeg2enc_vid_stream *mpeg2enc_new_encoder();
int mpeg2enc_new_picture (mpeg2enc_vid_stream *vid_stream, char *inbuf, int size, int encoder_state);

/* readpic.c */
void convertRGBtoYUV(mpeg2enc_vid_stream *vid_stream, unsigned char *source, unsigned char *dest);
