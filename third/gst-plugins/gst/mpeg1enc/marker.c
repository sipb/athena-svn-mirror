/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/

/*
************************************************************
marker.c

This file contains most of the marker information.

************************************************************
*/

/*LABEL marker.c */

#include "globals.h"
#include "marker.h"

static void CodeMV(mpeg1encoder_VidStream *vid_stream, int d, int fd);
static int DecodeMV(mpeg1encoder_VidStream *vid_stream, int fd, int oldvect);

#define Zigzag(i) izigzag_index[i]
const static int izigzag_index[] =
{0,  1,  8, 16,  9,  2,  3, 10,
17, 24, 32, 25, 18, 11,  4,  5,
12, 19, 26, 33, 40, 48, 41, 34,
27, 20, 13,  6,  7, 14, 21, 28,
35, 42, 49, 56, 57, 50, 43, 36,
29, 22, 15, 23, 30, 37, 44, 51,
58, 59, 52, 45, 38, 31, 39, 46,
53, 60, 61, 54, 47, 55, 62, 63};

/*START*/

/*BFUNC

ByteAlign() aligns the current stream to a byte-flush boundary. This
is used in the standard, with the assumption that input device is
byte-buffered.

EFUNC*/

void ByteAlign(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("ByteAlign"); */

  zeroflush(vid_stream);
}

/*BFUNC

WriteVEHeader() writes out a video sequence end marker.

EFUNC*/

void WriteVEHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("WriteVEHeader");*/

  ByteAlign(vid_stream);
  mputv(vid_stream,MBSC_LENGTH,MBSC);
  mputv(vid_stream,VSEC_LENGTH,VSEC);
}


/*BFUNC

WriteVSHeader() writes out a video sequence start marker.  Note that
Brate and Bsize are defined automatically by this routine. 

EFUNC*/

void WriteVSHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("WriteVSHeader");*/
  int i;

  ByteAlign(vid_stream);
  mputv(vid_stream,MBSC_LENGTH,MBSC);
  mputv(vid_stream,VSSC_LENGTH,VSSC);
  mputv(vid_stream,12,vid_stream->seq.HorizontalSize);
  mputv(vid_stream,12,vid_stream->seq.VerticalSize);
  mputv(vid_stream,4,vid_stream->seq.Aprat);
  if (vid_stream->XING) mputv(vid_stream,4,0x09);
  else mputv(vid_stream,4,vid_stream->seq.Prate);
  if (vid_stream->Rate) vid_stream->seq.Brate = (vid_stream->Rate+399)/400;         /* Round upward */
  if (vid_stream->XING)
    mputv(vid_stream,18,0x3ffff);
  else
    mputv(vid_stream,18,((vid_stream->seq.Brate!=0) ? vid_stream->seq.Brate : 0x3ffff));  /* Var-bit rate if Brate=0 */
  mputb(vid_stream,1);             /* Reserved bit */
  vid_stream->seq.Bsize=vid_stream->BufferSize/(16*1024);
  if (vid_stream->XING) mputv(vid_stream,10,16);
  else mputv(vid_stream,10,vid_stream->seq.Bsize);

  mputb(vid_stream,vid_stream->seq.ConstrainedParameterFlag);

  mputb(vid_stream,vid_stream->seq.LoadIntraQuantizerMatrix);
  if(vid_stream->seq.LoadIntraQuantizerMatrix)
    {
      for(i=0;i<64;i++)
	mputv(vid_stream,8,MPEGIntraQ[Zigzag(i)]);
    }

  mputb(vid_stream,vid_stream->seq.LoadNonIntraQuantizerMatrix);
  if(vid_stream->seq.LoadNonIntraQuantizerMatrix)
    {
      for(i=0;i<64;i++)
	mputv(vid_stream,8,MPEGNonIntraQ[Zigzag(i)]);
    }

  if (vid_stream->XING)
    {
      ByteAlign(vid_stream);
      mputv(vid_stream,32, 0x000001b2);
      mputv(vid_stream,8, 0x00);      /* number of frames? */
      mputv(vid_stream,8, 0x00);
      mputv(vid_stream,8, 0x02);      /* Other values 0x00e8 */
      mputv(vid_stream,8, 0xd0);
    }
}

/*BFUNC

ReadVSHeader() reads in the body of the video sequence start marker.

EFUNC*/

int ReadVSHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("ReadVSHeader");
  int i;

  vid_stream->seq.HorizontalSize = mgetv(vid_stream,12);
  vid_stream->seq.VerticalSize = mgetv(vid_stream,12);
  vid_stream->seq.Aprat = mgetv(vid_stream,4);
  if ((!vid_stream->seq.Aprat)||(vid_stream->seq.Aprat==0xf))
    {
      WHEREAMI();
      printf("Aspect ratio ill defined: %d.\n",vid_stream->seq.Aprat);
    }
  vid_stream->seq.Prate = mgetv(vid_stream,4);
  if ((!vid_stream->seq.Prate) || (vid_stream->seq.Prate > 8))
    {
      WHEREAMI();
      printf("Bad picture rate definition: %d\n",vid_stream->seq.Prate);
      vid_stream->seq.Prate = 6;
    }
  vid_stream->seq.Brate = mgetv(vid_stream,18);
  if (!vid_stream->seq.Brate)
    {
      WHEREAMI();
      printf("Illegal bit rate: %d.\n",vid_stream->seq.Brate);
    }
  if (vid_stream->seq.Brate == 0x3ffff)
    vid_stream->Rate=0;
  else
    vid_stream->Rate = vid_stream->seq.Brate*400;
      
  (void) mgetb(vid_stream);             /* Reserved bit */

  vid_stream->seq.Bsize = mgetv(vid_stream,10);
  vid_stream->BufferSize = vid_stream->seq.Bsize*16*1024;

  vid_stream->seq.ConstrainedParameterFlag = mgetb(vid_stream);

  vid_stream->seq.LoadIntraQuantizerMatrix = mgetb(vid_stream);
  if(vid_stream->seq.LoadIntraQuantizerMatrix)
    {
      for(i=0;i<64;i++)
	MPEGIntraQ[Zigzag(i)]= mgetv(vid_stream,8);
    }
  vid_stream->seq.LoadNonIntraQuantizerMatrix = mgetb(vid_stream);
  if(vid_stream->seq.LoadNonIntraQuantizerMatrix)
    {
      for(i=0;i<64;i++)
	MPEGNonIntraQ[Zigzag(i)] = mgetv(vid_stream,8);
    }
  return(0);
}

/*BFUNC

WriteGOPHeader() write a group of pictures header. Note that
the TimeCode variable needs to be defined correctly.

EFUNC*/

void WriteGOPHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("WriteGOPHeader");*/

  ByteAlign(vid_stream);
  mputv(vid_stream,MBSC_LENGTH,MBSC);
  mputv(vid_stream,GOP_LENGTH,GOPSC);
  mputv(vid_stream,25,vid_stream->gop.TimeCode);
  mputb(vid_stream,vid_stream->gop.ClosedGOP);
  mputb(vid_stream,vid_stream->gop.BrokenLink);
}

/*BFUNC

ReadGOPHeader() reads the body of the group of pictures marker.

EFUNC*/

void ReadGOPHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("ReadGOPHeader");*/

  vid_stream->gop.TimeCode = mgetv(vid_stream,25);
  vid_stream->gop.ClosedGOP = mgetb(vid_stream);
  vid_stream->gop.BrokenLink = mgetb(vid_stream);
}

/*BFUNC

WritePictureHeader() writes the header of picture out to the stream.
One of these is necessary before every frame is transmitted.

EFUNC*/

void WritePictureHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("WritePictureHeader");
  static int frame=1;

  ByteAlign(vid_stream);
  mputv(vid_stream,MBSC_LENGTH,MBSC);
  mputv(vid_stream,PSC_LENGTH,PSC);
  if (vid_stream->XING) mputv(vid_stream,10,frame++);
  else mputv(vid_stream,10,vid_stream->pic.TemporalReference);
  mputv(vid_stream,3,vid_stream->pic.PType);

  if (vid_stream->XING) mputv(vid_stream,16,0xffff);
  else
    {
      if (vid_stream->pic.BufferFullness<0)
	{
	  WHEREAMI();
	  printf("Virtual decoder buffer fullness less than zero.\n");
	  mputv(vid_stream,16,0);
	}
      else if (vid_stream->pic.BufferFullness > 65535)
	{
	  WHEREAMI();
	  printf("Virtual decoder buffer fullness > 65536/90000 second.\n");
	  mputv(vid_stream,16,0xffff);
	}
      else
	mputv(vid_stream,16,vid_stream->pic.BufferFullness);
    }


  if ((vid_stream->pic.PType == P_PREDICTED) || (vid_stream->pic.PType == P_INTERPOLATED))
    {
      mputb(vid_stream,vid_stream->pic.FullPelForward);
      mputv(vid_stream,3,vid_stream->pic.ForwardIndex);
    }
  if (vid_stream->pic.PType == P_INTERPOLATED)
    {
      mputb(vid_stream,vid_stream->pic.FullPelBackward);
      mputv(vid_stream,3,vid_stream->pic.BackwardIndex);
    }
  if (vid_stream->XING)
    {
      mputb(vid_stream,1);
      mputv(vid_stream,8,0xff);
      mputb(vid_stream,1);
      mputv(vid_stream,8,0xfe);
      ByteAlign(vid_stream);           /* The bytealign seems to work well here */
      mputv(vid_stream,32, 0x000001b2);
      mputv(vid_stream,8, 0xff);
      mputv(vid_stream,8, 0xff);
    }
  else
    {
      mputb(vid_stream,vid_stream->pic.PictureExtra);
      if (vid_stream->pic.PictureExtra)
	{
	  mputv(vid_stream,8,vid_stream->pic.PictureExtraInfo);
	  mputb(vid_stream,0);
	}
    }
}

/*BFUNC

ReadPictureHeader() reads the header off of the stream. It assumes
that the first PSC has already been read in. (Necessary to tell the
difference between a new picture and another GOP.)

EFUNC*/

void ReadPictureHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("ReadPictureHeader");

  vid_stream->pic.TemporalReference = mgetv(vid_stream,10);
  vid_stream->pic.PType = mgetv(vid_stream,3);
  if (!vid_stream->pic.PType)
    {
      WHEREAMI();
      printf("Illegal PType received.\n");
    }
  vid_stream->pic.BufferFullness = mgetv(vid_stream,16);

  if ((vid_stream->pic.PType == P_PREDICTED) || (vid_stream->pic.PType == P_INTERPOLATED))
    {
      vid_stream->pic.FullPelForward = mgetb(vid_stream);
      vid_stream->pic.ForwardIndex = mgetv(vid_stream,3);
    }
  if (vid_stream->pic.PType == P_INTERPOLATED)
    {
      vid_stream->pic.FullPelBackward = mgetb(vid_stream);
      vid_stream->pic.BackwardIndex = mgetv(vid_stream,3);
    }
  vid_stream->pic.PictureExtra=0;
  while(mgetb(vid_stream))
    {
      vid_stream->pic.PictureExtraInfo = mgetv(vid_stream,8);
      vid_stream->pic.PictureExtra = 1;
    }
}

/*BFUNC

WriteMBSHeader() writes a macroblock slice header out to the stream.

EFUNC*/

void WriteMBSHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("WriteMBSHeader");*/

  ByteAlign(vid_stream);
  mputv(vid_stream,MBSC_LENGTH,MBSC);
  /* printf("Wrote: MBS-> SVP: %d\n",SVP);*/
  mputv(vid_stream,8,vid_stream->mb.SVP);
  mputv(vid_stream,5,vid_stream->slice.SQuant);
  if (vid_stream->slice.SliceExtra)
    {
      mputb(vid_stream,1);
      mputv(vid_stream,8,vid_stream->slice.SliceExtraInfo);
    }
  mputb(vid_stream,0);
}

/*BFUNC

ReadMBSHeader() reads the slice information off of the stream. We
assume that the first bits have been read in by ReadHeaderHeader... or
some such routine.

EFUNC*/

void ReadMBSHeader(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("ReadMBSHeader");*/

  vid_stream->slice.SQuant = mgetv(vid_stream,5);
  for(vid_stream->slice.SliceExtra=0;mgetb(vid_stream);)
    {
      vid_stream->slice.SliceExtraInfo = mgetv(vid_stream,8);
      vid_stream->slice.SliceExtra = 1;
    }
}

/*BFUNC

ReadHeaderTrailer(GOP,)
     ) reads the trailer of the GOP; reads the trailer of the GOP, PSC or MBSC code. It
is used to determine whether it is just a new group of frames, new
picture, or new slice.

EFUNC*/

void ReadHeaderTrailer(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("ReadHeaderTrailer");*/

  while(1)
    {
      vid_stream->TrailerValue = mgetv(vid_stream,8);
      if (!vid_stream->TrailerValue)              /* Start of picture */
	{
	  vid_stream->mb.MBSRead = -1;
	  break;
	}
      else if (vid_stream->TrailerValue==GOPSC)   /* Start of group of picture */
	{
	  vid_stream->mb.MBSRead = -2;
	  break;
	}
      else if (vid_stream->TrailerValue==VSEC)   /* Start of group of picture */
	{
	  vid_stream->mb.MBSRead = -3;
	  break;
	}
      else if (vid_stream->TrailerValue==VSSC)
	{
	  vid_stream->mb.MBSRead = -4;
	  break;
	}
      else if ((vid_stream->TrailerValue > 0) && (vid_stream->TrailerValue < 0xb0)) /* Slice vp */
	{
	  vid_stream->mb.MBSRead = vid_stream->TrailerValue-1;
	  vid_stream->mb.SVP = vid_stream->TrailerValue;
	  /* printf("Read SVP %d\n",SVP);*/
	  break;
	}
      else if (vid_stream->TrailerValue == UDSC)
	{
	  printf("User data code found.\n");
	  ClearToHeader(vid_stream);
	}
      else if (vid_stream->TrailerValue == EXSC)
	{
	  printf("Extension data code found.\n");
	  ClearToHeader(vid_stream);
	}
      else
	break;
    }
}

/*BFUNC

ReadHeaderHeader() reads the common structure header series of bytes
(and alignment) off of the stream.  This is a precursor to the GOP
read or the PSC read. It returns -1 on error.

EFUNC*/

int ReadHeaderHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("ReadHeaderHeader");
  int input;

  readalign(vid_stream);                /* Might want to check if all 0's */
  if ((input = mgetv(vid_stream,MBSC_LENGTH)) != MBSC)
    {
      while(!input)
	{
	  if ((input = mgetv(vid_stream,8)) == MBSC)  /* Get next byte */
	    return(0);
	  else if (input && (seof(vid_stream)))
	    {
	      WHEREAMI();
	      printf("End of file.\n");
	    }
	}
      WHEREAMI();
      printf("Bad input read: %d\n",input);
      return(-1);
    }
  return(0);
}

/*BFUNC

ClearToHeader() reads the header header off of the stream. This is
a precursor to the GOP read or the PSC read. It returns -1 on error.

EFUNC*/

int ClearToHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("ReadHeaderHeader");
  int input;

  readalign(vid_stream);                /* Might want to check if all 0's */
  if ((input = mgetv(vid_stream,MBSC_LENGTH)) != MBSC)
    {
      do
	{
	  if (seof(vid_stream))
	    {
	      WHEREAMI();
	      printf("Illegal termination.\n");
/* FIXME */
/*	      exit(-1); */
	    }
	  input = input & 0xffff;               /* Shift off by 8 */
	  input = (input << 8) | mgetv(vid_stream,8);
	}
      while (input != MBSC);  /* Get marker */
    }
  return(0);
}

/*BFUNC

WriteStuff() writes a MB stuff code. 

EFUNC*/

void WriteStuff(mpeg1encoder_VidStream *vid_stream)
{
  /*BEGIN("WriteStuff");*/
  mputv(vid_stream,11,0xf);
}

/*BFUNC

WriteMBHeader() writes the macroblock header information out to the stream.

EFUNC*/

void WriteMBHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("WriteMBHeader");
  int i,TempH,TempV,Start,retval=0;

  /* printf("[MBAInc: %d]",MBAIncrement); */
  /* printf("[Writing: %d]\n",MType); */
#ifdef MV_DEBUG
#endif
  Start=swtell(vid_stream);
  if (vid_stream->mb.MBAIncrement > 33)
    {
      for(i=0;i<((vid_stream->mb.MBAIncrement-1)/33);i++)
	{
	  if (!Encode(vid_stream,35,vid_stream->huff.MBAEHuff)) /* Escape code */
	    {
	      WHEREAMI();
	      printf("Attempting to write an empty Huffman code (35).\n");
/* FIXME */
/*	      exit(ERROR_HUFFMAN_ENCODE); */
	    }
	}
      if (!Encode(vid_stream,((vid_stream->mb.MBAIncrement-1)%33)+1,vid_stream->huff.MBAEHuff))
	{
	  WHEREAMI();
	  printf("Attempting to write an empty Huffman code (%d).\n",
		 (vid_stream->mb.MBAIncrement-1)%33);
/* FIXME */
/*	  exit(ERROR_HUFFMAN_ENCODE); */
	}
    }
  else
    {
      if (!Encode(vid_stream,vid_stream->mb.MBAIncrement,vid_stream->huff.MBAEHuff))
	{
	  WHEREAMI();
	  printf("Attempting to write an empty Huffman code (%d).\n",
		 vid_stream->mb.MBAIncrement);
/* FIXME */
/*	  exit(ERROR_HUFFMAN_ENCODE); */
	}
    }
  switch(vid_stream->pic.PType)
    {
    case P_INTRA:
      retval=Encode(vid_stream,vid_stream->mb.MType,vid_stream->huff.IntraEHuff);
      break;
    case P_PREDICTED:
      retval=Encode(vid_stream,vid_stream->mb.MType,vid_stream->huff.PredictedEHuff);
      break;
    case P_INTERPOLATED:
      retval=Encode(vid_stream,vid_stream->mb.MType,vid_stream->huff.InterpolatedEHuff);
      break;
    case P_DCINTRA:
      mputb(vid_stream,1);           /* only one type for DC Intra */
      retval=1;
      break;
    default:
      WHEREAMI();
      printf("Bad picture type: %d\n",vid_stream->pic.PType);
      break;
    }
  if (!retval)
    {
      WHEREAMI();
      printf("Attempting to write an empty Huffman code.\n");
/* FIXME */
/*      exit(ERROR_HUFFMAN_ENCODE); */
    }
  vid_stream->NumberBitsCoded=0;
  if (QuantPMType[vid_stream->pic.PType][vid_stream->mb.MType]) mputv(vid_stream,5,vid_stream->mb.MQuant);
  if (MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      TempH = vid_stream->mb.MVD1H - vid_stream->mb.LastMVD1H;
      TempV = vid_stream->mb.MVD1V - vid_stream->mb.LastMVD1V;

#ifdef MV_DEBUG
      printf("1st FI: %d  Type: %d  Actual: H %d  V %d  Coding: H %d  V %d\n",
	     vid_stream->pic.ForwardIndex,vid_stream->mb.MType,vid_stream->mb.MVD1H,vid_stream->mb.MVD1V,TempH,TempV);
#endif
      if (vid_stream->pic.FullPelForward)
	{
	  CodeMV(vid_stream,TempH/2,vid_stream->pic.ForwardIndex);
	  CodeMV(vid_stream,TempV/2,vid_stream->pic.ForwardIndex);
	}
      else
	{
	  CodeMV(vid_stream,TempH,vid_stream->pic.ForwardIndex);
	  CodeMV(vid_stream,TempV,vid_stream->pic.ForwardIndex);
	}
      vid_stream->mb.LastMVD1V = vid_stream->mb.MVD1V;
      vid_stream->mb.LastMVD1H = vid_stream->mb.MVD1H;
    }
  if (MBPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      TempH = vid_stream->mb.MVD2H - vid_stream->mb.LastMVD2H;
      TempV = vid_stream->mb.MVD2V - vid_stream->mb.LastMVD2V;

#ifdef MV_DEBUG
      printf("2nd BI: %d  Type: %d  Actual: H %d  V %d  Coding: H %d  V %d\n",
             vid_stream->pic.BackwardIndex,vid_stream->mb.MType,vid_stream->mb.MVD2H,vid_stream->mb.MVD2V,TempH,TempV);
#endif
      if (vid_stream->pic.FullPelBackward)
	{
	  CodeMV(vid_stream,TempH/2,vid_stream->pic.BackwardIndex);
	  CodeMV(vid_stream,TempV/2,vid_stream->pic.BackwardIndex);
	}
      else
	{
	  CodeMV(vid_stream,TempH,vid_stream->pic.BackwardIndex);
	  CodeMV(vid_stream,TempV,vid_stream->pic.BackwardIndex);
	}
      vid_stream->mb.LastMVD2V = vid_stream->mb.MVD2V;
      vid_stream->mb.LastMVD2H = vid_stream->mb.MVD2H;
    }
  vid_stream->MotionVectorBits+=vid_stream->NumberBitsCoded;

  if (CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      /* printf("CBP: %d\n",CBP); */
      if (!Encode(vid_stream,vid_stream->mb.CBP,vid_stream->huff.CBPEHuff))
	{
	  WHEREAMI();
	  printf("CBP write error: PType: %d  MType: %d CBP: %d.\n",
		 vid_stream->pic.PType,vid_stream->mb.MType,vid_stream->mb.CBP);
/* FIXME */
/*	  exit(-1); */
	}
    }
  vid_stream->MacroAttributeBits+=(swtell(vid_stream)-Start);
}

static void CodeMV(vid_stream, d,fd)
     mpeg1encoder_VidStream *vid_stream;
     int d;
     int fd;
{
  BEGIN("CodeMV");
  int v,limit;
  int fcode, motion_r;

  if (!d)
    {
      /* printf("Encoding zero\n"); */
      if (!Encode(vid_stream,0,vid_stream->huff.MVDEHuff))
	{
	  WHEREAMI();
	  printf("Cannot encode motion vectors.\n");
	}
      return;
    }
  limit = 1 << (fd+3); /* find limit 16*(1<<(fd-1)) */
  /* Calculate modulo factors */
  if (d < -limit) /* Do clipping */
    {
      d += (limit << 1);
      if (d <= 0)
	{
	  WHEREAMI();
	  printf("Motion vector out of bounds: (residual) %d\n",d);
	}
    }
  else if (d>=limit)
    {
      d -= (limit << 1);
      if (d >= 0)
	{
	  WHEREAMI();
	  printf("Motion vector out of bounds: (residual) %d\n",d);
	}
    }
  fd--;
  if (fd)
    {
      if (!d)
	{
	  if (!Encode(vid_stream,d,vid_stream->huff.MVDEHuff))
	    {
	      WHEREAMI();
	      printf("Cannot encode zero motion vector.\n");
	    }
	  return;
	}
      if (d > 0)
	{
	  d--;                           /* Dead band at zero */
	  fcode = (d>>fd)+1;             /* Quantize, move up */
	  motion_r = d&((1<<fd)-1);      /* use lowest fd bits */
	}                                /* May not need mask for mputv */
      else
	{
	  fcode = d>>fd;                 /* negative, dead band auto. */
	  motion_r = (-1^d)&((1<<fd)-1); /* lowest fd bits of (abs(d)-1)*/
	}                                /* May not need mask for mputv */
      if (fcode < 0) v = 33+fcode;       /* We have positive array index */
      else v = fcode;
      if (!Encode(vid_stream,v,vid_stream->huff.MVDEHuff))
	{
	  WHEREAMI();
	  printf("Cannot encode motion vectors.\n");
	}
      mputv(vid_stream,fd,motion_r);
    }
  else
    {
      if (d < 0) v = 33+d;         /* Compensate for positive array index */
      else v = d;
      if (!Encode(vid_stream,v,vid_stream->huff.MVDEHuff))
	{
	  WHEREAMI();
	  printf("Cannot encode motion vectors.\n");
	}
    }
}

static int DecodeMV(vid_stream,fd,oldvect)
     mpeg1encoder_VidStream *vid_stream;
     int fd;
     int oldvect;
{
  BEGIN("DecodeMV");
  int r,v,limit;
  
  /* fd is the frame displacement */
  /* limit is the maximum displacement range we can have = 16*(1<<(fd-1)) */
  limit = 1 << (fd+3); 
  v = Decode(vid_stream,vid_stream->huff.MVDDHuff);
  if (v)
    {
      if (v > 16) {v = v-33;}       /* our codes are positive, negative vals */
                                    /* are greater than 16. */
      fd--;                         /*Find out number of modulo bits=fd */
      if (fd)
	{
	  r = mgetv(vid_stream,fd);            /* modulo lower bits */
	  if (v > 0) 
	    v = (((v-1)<<fd)|r)+1;  /* just "or" and add 1 for dead band. */
	  else
	    v = ((v+1)<<fd)+(-1^r); /* Needs mask to do an "or".*/
	}
      if (v==limit)
	{
	  WHEREAMI();
	  printf("Warning: motion vector at positive limit.\n");
	}
    }
  v += oldvect;                      /* DPCM */
  if (v < -limit)
    v += (limit << 1);
  else if (v >= limit)
    v -= (limit << 1);
  if (v==limit)
    {
      WHEREAMI();
      printf("Apparently illegal reference: (MV %d) (LastMV %d).\n",v,oldvect);
    }
  return(v);
}

/*BFUNC

ReadMBHeader() reads the macroblock header information from the stream.

EFUNC*/

int ReadMBHeader(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("ReadMBHeader");
  int Readin;

  for(vid_stream->mb.MBAIncrement=0;;)
    {
      do
	{
	  Readin = Decode(vid_stream,vid_stream->huff.MBADHuff);
	}
      while(Readin == 34);  /* Get rid of stuff bits */
      if (Readin <34)
	{
	  vid_stream->mb.MBAIncrement += Readin;
	  break;
	}
      else if (Readin == 36)
	{
	  while(!mgetb(vid_stream));
	  return(-1); /* Start of Picture Headers */
	}
      else if (Readin == 35)
	vid_stream->mb.MBAIncrement += 33;
      else
	{
	  WHEREAMI();
	  printf("Bad MBA Read: %d \n",Readin);
	  break;
	}
    }
  /* printf("[MBAInc: %d]\n",MBAIncrement); */
  switch(vid_stream->pic.PType)
    {
    case P_INTRA:
      vid_stream->mb.MType = Decode(vid_stream,vid_stream->huff.IntraDHuff);
      break;
    case P_PREDICTED:
      if (vid_stream->mb.MBAIncrement > 1) vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=0; /* Erase MV for skipped mbs */
      vid_stream->mb.MType = Decode(vid_stream,vid_stream->huff.PredictedDHuff);
      break;
    case P_INTERPOLATED:
      vid_stream->mb.MType = Decode(vid_stream,vid_stream->huff.InterpolatedDHuff);
      break;
    case P_DCINTRA:
      if (!mgetb(vid_stream))
	{
	  WHEREAMI();
	  printf("Expected one bit for DC Intra, 0 read.\n");
	}
      break;
    default:
      WHEREAMI();
      printf("Bad picture type.\n");
      break;
    }
#ifdef MV_DEBUG
  printf("[Reading: %d]",vid_stream->mb.MType);
#endif
  if (QuantPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.MQuant=mgetv(vid_stream,5);
  if (MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      if (vid_stream->pic.FullPelForward)
	{
	  vid_stream->mb.MVD1H = DecodeMV(vid_stream,vid_stream->pic.ForwardIndex,vid_stream->mb.MVD1H/2)<<1;
	  vid_stream->mb.MVD1V = DecodeMV(vid_stream,vid_stream->pic.ForwardIndex,vid_stream->mb.MVD1V/2)<<1;
	}
      else
	{
	  vid_stream->mb.MVD1H = DecodeMV(vid_stream,vid_stream->pic.ForwardIndex,vid_stream->mb.MVD1H);
	  vid_stream->mb.MVD1V = DecodeMV(vid_stream,vid_stream->pic.ForwardIndex,vid_stream->mb.MVD1V);
	}
#ifdef MV_DEBUG
      printf("1st FI: %d  Type: %d  Actual: H %d  V %d\n",
	     vid_stream->pic.ForwardIndex,vid_stream->mb.MType,vid_stream->mb.MVD1H,vid_stream->mb.MVD1V);
#endif
    }
  else if (vid_stream->pic.PType==P_PREDICTED)
    vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=0;

  if (MBPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      if (vid_stream->pic.FullPelBackward)
	{
	  vid_stream->mb.MVD2H = DecodeMV(vid_stream,vid_stream->pic.BackwardIndex,vid_stream->mb.MVD2H/2)<<1;
	  vid_stream->mb.MVD2V = DecodeMV(vid_stream,vid_stream->pic.BackwardIndex,vid_stream->mb.MVD2V/2)<<1;
	}
      else
	{
	  vid_stream->mb.MVD2H = DecodeMV(vid_stream,vid_stream->pic.BackwardIndex,vid_stream->mb.MVD2H);
	  vid_stream->mb.MVD2V = DecodeMV(vid_stream,vid_stream->pic.BackwardIndex,vid_stream->mb.MVD2V);
	}
#ifdef MV_DEBUG
      printf("2nd BI: %d  Type: %d   Actual: H %d  V %d.\n",
	     vid_stream->pic.BackwardIndex,vid_stream->mb.MType,vid_stream->mb.MVD2H,vid_stream->mb.MVD2V);
#endif
    }
  if (CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.CBP = Decode(vid_stream,vid_stream->huff.CBPDHuff);
  else if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.CBP=0x3f;
  else vid_stream->mb.CBP=0;

  /* printf("CBP: %d\n",CBP);*/
  return(0);
}

/*END*/

