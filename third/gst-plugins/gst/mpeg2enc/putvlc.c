/* putvlc.c, generation of variable length codes                            */

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
#include "vlc.h"

/* private prototypes */
static void putDC _ANSI_ARGS_((mpeg2enc_vid_stream *vid_stream, sVLCtable *tab, int val));

/* generate variable length code for luminance DC coefficient */
void putDClum(vid_stream,val)
mpeg2enc_vid_stream *vid_stream;
int val;
{
  putDC(vid_stream,DClumtab,val);
}

/* generate variable length code for chrominance DC coefficient */
void putDCchrom(vid_stream,val)
mpeg2enc_vid_stream *vid_stream;
int val;
{
  putDC(vid_stream,DCchromtab,val);
}

/* generate variable length code for DC coefficient (7.2.1) */
static void putDC(vid_stream,tab,val)
mpeg2enc_vid_stream *vid_stream;
sVLCtable *tab;
int val;
{
  int absval, size;

  absval = (val<0) ? -val : val; /* abs(val) */

  if (absval>2047 || (vid_stream->mpeg1 && absval>255))
  {
    /* should never happen */
    sprintf(vid_stream->errortext,"DC value out of range (%d)\n",val);
    error(vid_stream->errortext);
  }

  /* compute dct_dc_size */
  size = 0;

  while (absval)
  {
    absval >>= 1;
    size++;
  }

  /* generate VLC for dct_dc_size (Table B-12 or B-13) */
  gst_putbits(&vid_stream->pb,tab[size].code,tab[size].len);

  /* append fixed length code (dc_dct_differential) */
  if (size!=0)
  {
    if (val>=0)
      absval = val;
    else
      absval = val + (1<<size) - 1; /* val + (2 ^ size) - 1 */
    gst_putbits(&vid_stream->pb,absval,size);
  }
}

/* generate variable length code for first coefficient
 * of a non-intra block (7.2.2.2) */
void putACfirst(vid_stream,run,val)
mpeg2enc_vid_stream *vid_stream;
int run,val;
{
  if (run==0 && (val==1 || val==-1)) /* these are treated differently */
    gst_putbits(&vid_stream->pb,2|(val<0),2); /* generate '1s' (s=sign), (Table B-14, line 2) */
  else
    putAC(vid_stream,run,val,0); /* no difference for all others */
}

/* generate variable length code for other DCT coefficients (7.2.2) */
void putAC(vid_stream,run,signed_level,vlcformat)
mpeg2enc_vid_stream *vid_stream;
int run,signed_level,vlcformat;
{
  int level, len;
  VLCtable *ptab;

  level = (signed_level<0) ? -signed_level : signed_level; /* abs(signed_level) */

  /* make sure run and level are valid */
  if (run<0 || run>63 || level==0 || level>2047 || (vid_stream->mpeg1 && level>255))
  {
    sprintf(vid_stream->errortext,"AC value out of range (run=%d, signed_level=%d)\n",
      run,signed_level);
    error(vid_stream->errortext);
  }

  if (run<2 && level<41)
  {
    /* vlcformat selects either of Table B-14 / B-15 */
    if (vlcformat)
      ptab = &dct_code_tab1a[run][level-1];
    else
      ptab = &dct_code_tab1[run][level-1];

    len = ptab->len;
  }
  else if (run<32 && level<6)
  {
    /* vlcformat selects either of Table B-14 / B-15 */
    if (vlcformat)
      ptab = &dct_code_tab2a[run-2][level-1];
    else
      ptab = &dct_code_tab2[run-2][level-1];

    len = ptab->len;
  }
  else
  {
    ptab = NULL;
    len = 0;
  }

  if (len!=0) /* a VLC code exists */
  {
    gst_putbits(&vid_stream->pb,ptab->code,len);
    gst_putbits(&vid_stream->pb,signed_level<0,1); /* sign */
  }
  else
  {
    /* no VLC for this (run, level) combination: use escape coding (7.2.2.3) */
    gst_putbits(&vid_stream->pb,1l,6); /* Escape */
    gst_putbits(&vid_stream->pb,run,6); /* 6 bit code for run */
    if (vid_stream->mpeg1)
    {
      /* ISO/IEC 11172-2 uses a 8 or 16 bit code */
      if (signed_level>127)
        gst_putbits(&vid_stream->pb,0,8);
      if (signed_level<-127)
        gst_putbits(&vid_stream->pb,128,8);
      gst_putbits(&vid_stream->pb,signed_level,8);
    }
    else
    {
      /* ISO/IEC 13818-2 uses a 12 bit code, Table B-16 */
      gst_putbits(&vid_stream->pb,signed_level,12);
    }
  }
}

/* generate variable length code for macroblock_address_increment (6.3.16) */
void putaddrinc(vid_stream,addrinc)
mpeg2enc_vid_stream *vid_stream;
int addrinc;
{
  while (addrinc>33)
  {
    gst_putbits(&vid_stream->pb,0x08,11); /* macroblock_escape */
    addrinc-= 33;
  }

  gst_putbits(&vid_stream->pb,addrinctab[addrinc-1].code,addrinctab[addrinc-1].len);
}

/* generate variable length code for macroblock_type (6.3.16.1) */
void putmbtype(vid_stream,pict_type,mb_type)
mpeg2enc_vid_stream *vid_stream;
int pict_type,mb_type;
{
  gst_putbits(&vid_stream->pb,mbtypetab[pict_type-1][mb_type].code,
          mbtypetab[pict_type-1][mb_type].len);
}

/* generate variable length code for motion_code (6.3.16.3) */
void putmotioncode(vid_stream,motion_code)
mpeg2enc_vid_stream *vid_stream;
int motion_code;
{
  int abscode;

  abscode = (motion_code>=0) ? motion_code : -motion_code; /* abs(motion_code) */
  gst_putbits(&vid_stream->pb,motionvectab[abscode].code,motionvectab[abscode].len);
  if (motion_code!=0)
    gst_putbits(&vid_stream->pb,motion_code<0,1); /* sign, 0=positive, 1=negative */
}

/* generate variable length code for dmvector[t] (6.3.16.3), Table B-11 */
void putdmv(vid_stream,dmv)
mpeg2enc_vid_stream *vid_stream;
int dmv;
{
  if (dmv==0)
    gst_putbits(&vid_stream->pb,0,1);
  else if (dmv>0)
    gst_putbits(&vid_stream->pb,2,2);
  else
    gst_putbits(&vid_stream->pb,3,2);
}

/* generate variable length code for coded_block_pattern (6.3.16.4)
 *
 * 4:2:2, 4:4:4 not implemented
 */
void putcbp(vid_stream,cbp)
mpeg2enc_vid_stream *vid_stream;
int cbp;
{
  gst_putbits(&vid_stream->pb,cbptable[cbp].code,cbptable[cbp].len);
}
