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
codec.c

This file contains the routines to run-length encode the ac and dc
coefficients.

************************************************************
*/

/*LABEL codec.c */

#include "globals.h"
#include "stream.h"
#include "csize.h"

#define fputv mputv
#define fgetv mgetv
#define fputb mputb
#define fgetb mgetb

static const int extend_mask[] = {
0xFFFFFFFE,
0xFFFFFFFC,
0xFFFFFFF8,
0xFFFFFFF0,
0xFFFFFFE0,
0xFFFFFFC0,
0xFFFFFF80,
0xFFFFFF00,
0xFFFFFE00,
0xFFFFFC00,
0xFFFFF800,
0xFFFFF000,
0xFFFFE000,
0xFFFFC000,
0xFFFF8000,
0xFFFF0000,
0xFFFE0000,
0xFFFC0000,
0xFFF80000,
0xFFF00000
};

/*START*/

/*BFUNC

EncodeAC() encodes the quantized coefficient matrix input by the first
Huffman table.  The index is an offset into the matrix.

EFUNC*/

void EncodeAC(vid_stream, index,matrix)
     mpeg1encoder_VidStream *vid_stream;
     int index;
     int *matrix;
{
  /*BEGIN("EncodeAC");*/
  int k,r,l,code,retval,Start;

  Start=swtell(vid_stream);
  for(r=0,k=index-1;++k<BLOCKSIZE;)
    {
      l = matrix[k];
      if (!l) {r++;}
      else
	{
	  if ((l < 128) && (l > -128))
	    {
	      code = abs(l) | (r << 8);
	      if (code != HUFFMAN_ESCAPE)
		{
		  retval=Encode(vid_stream,code,vid_stream->huff.T1EHuff);
		}
	      else {retval=0;}
	      if (!retval)
		{
		  Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T1EHuff);
		  fputv(vid_stream,6,r);
		  fputv(vid_stream,8,l);
		}
	      else
		{
		  if (l < 0){fputb(vid_stream,1);}
		  else {fputb(vid_stream,0);}
		}
	    }
	  else
	    {
	      Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T1EHuff);
	      fputv(vid_stream,6,r);
	      if (l>0) {fputv(vid_stream,8,0x00);}
	      else {fputv(vid_stream,8,0x80);} /* Negative */
	      fputv(vid_stream,8,l&0xff);
	    }
	  r=0;
	  vid_stream->NumberNZ++;
	}
    }
  vid_stream->CodedBlockBits+=(swtell(vid_stream)-Start);
  vid_stream->EOBBits+=Encode(vid_stream,0,vid_stream->huff.T1EHuff);
}

/*BFUNC

CBPEncodeAC() encodes the AC block matrix when we know there exists a
non-zero coefficient in the matrix. Thus the EOB cannot occur as the
first element and we save countless bits...

EFUNC*/

void CBPEncodeAC(vid_stream, index,matrix)
     mpeg1encoder_VidStream *vid_stream;
     int index;
     int *matrix;
{
  BEGIN("CBPEncodeAC");
  int k,r,l,code,retval,ovfl,Start;

  Start=swtell(vid_stream);
  for(ovfl=1,r=0,k=index-1;++k<BLOCKSIZE;)
    {
      l = matrix[k];
      if (!l) {r++;}
      else
	{
	  if ((l < 128) && (l > -128))
	    {
	      code = abs(l) | (r << 8);
	      if (code != HUFFMAN_ESCAPE)
		retval=Encode(vid_stream,code,vid_stream->huff.T2EHuff);
	      else 
		retval=0;
	      if (!retval)
		{
		  Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T2EHuff);
		  fputv(vid_stream,6,r);
		  fputv(vid_stream,8,l);
		}
	      else
		{
		  if (l < 0) {fputb(vid_stream,1);}
		  else {fputb(vid_stream,0);}
		}
	    }
	  else
	    {
	      Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T1EHuff);
	      fputv(vid_stream,6,r);
	      if (l>0) {fputv(vid_stream,8,0x00);}
	      else {fputv(vid_stream,8,0x80);} /* Negative */
	      fputv(vid_stream,8,l&0xff);
	    }
	  ovfl=0;
	  vid_stream->NumberNZ++;
	  break;
	}
    }
  if (ovfl)
    {
      WHEREAMI();
      printf("CBP block without any coefficients.\n");
      return;
    }
  for(r=0;++k<BLOCKSIZE;)
    {
      l = matrix[k];
      if (!l) {r++;}
      else
	{
	  if ((l < 128) && (l > -128))
	    {
	      code = abs(l) | (r << 8);
	      if (code != HUFFMAN_ESCAPE) {retval=Encode(vid_stream,code,vid_stream->huff.T1EHuff);}
	      else {retval=0;}
	      if (!retval)
		{
		  Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T1EHuff);
		  fputv(vid_stream,6,r);
		  fputv(vid_stream,8,l);
		}
	      else
		{
		  if (l < 0) {fputb(vid_stream,1);}
		  else {fputb(vid_stream,0);}
		}
	    }
	  else
	    {
	      Encode(vid_stream,HUFFMAN_ESCAPE,vid_stream->huff.T1EHuff);
	      fputv(vid_stream,6,r);
	      if (l>0) {fputv(vid_stream,8,0x00);}
	      else {fputv(vid_stream,8,0x80);} /* Negative */
	      fputv(vid_stream,8,l&0xff);
	    }
	  r=0;
	  vid_stream->NumberNZ++;
	}
    }
  vid_stream->CodedBlockBits+=(swtell(vid_stream)-Start);
  vid_stream->EOBBits+=Encode(vid_stream,0,vid_stream->huff.T1EHuff);
}

/*BFUNC

DecodeAC() decodes from the stream and puts the elements into the matrix
starting from the value of index. The matrix must be given as input and
initialized to a value at least as large as 64.

EFUNC*/

void DecodeAC(vid_stream, index,matrix)
     mpeg1encoder_VidStream *vid_stream;
     int index;
     int *matrix;
{
  BEGIN("DecodeAC");
  int k,r,l;
  int *mptr;

  for(mptr=matrix+index;mptr<matrix+BLOCKSIZE;mptr++) {*mptr = 0;}
  for(k=index;k<BLOCKSIZE;)  /* JPEG Mistake */
    {
      r = Decode(vid_stream,vid_stream->huff.T1DHuff);
      if (!r)
	{
#ifdef CODEC_DEBUG
	  printf("Return:%d\n",k);
#endif
	  return;
	} /*Eof*/
      if (r == HUFFMAN_ESCAPE)
	{
	  r = fgetv(vid_stream,6);
	  l = fgetv(vid_stream,8);
	  if (l==0x00)
	    l = fgetv(vid_stream,8); /* extended precision */
	  else if (l==0x80)
	    l = fgetv(vid_stream,8) | extend_mask[7]; /* extended sign */
	  else if (l & bit_set_mask[7]) 
	    l |= extend_mask[7];
	}
      else
	{
	  l = r & 0xff;
	  r = r >> 8;
	  if (fgetb(vid_stream)) {l = -l;}
	}
#ifdef CODEC_DEBUG
      printf("DecodeAC: k:%d r:%d l:%d\n",k,r,l);
#endif
      k += r;
      if (k>=BLOCKSIZE)
	{
	  WHEREAMI();
	  printf("k greater than blocksize:%d\n",k);
	  break;
	}
      matrix[k++] = l;
      vid_stream->NumberNZ++;
    }
  if ((r=Decode(vid_stream,vid_stream->huff.T1DHuff)))
    {
      WHEREAMI();
      printf("EOB expected, found 0x%x.\n",r);
    }
}

/*BFUNC

CBPDecodeAC() decodes the stream when we assume the input cannot begin
with an EOB Huffman code. Thus we use a different Huffman table. The
input index is the offset within the matrix and the matrix must
already be defined to be greater than 64 elements of int.

EFUNC*/

void CBPDecodeAC(vid_stream, index,matrix)
     mpeg1encoder_VidStream *vid_stream;
     int index;
     int *matrix;
{
  BEGIN("CBPDecodeAC");
  int k,r,l;
  int *mptr;

  for(mptr=matrix+index;mptr<matrix+BLOCKSIZE;mptr++) {*mptr = 0;}
  k = index;
  r = Decode(vid_stream,vid_stream->huff.T2DHuff);
  if (!r)
    {
      WHEREAMI();
      printf("Bad EOF in CBP block.\n");
      return;
    }
  if (r==HUFFMAN_ESCAPE)
    {
      r = fgetv(vid_stream,6);
      l = fgetv(vid_stream,8);
      if (l==0x00)
	l = fgetv(vid_stream,8); /* extended precision */
      else if (l==0x80)
	l = fgetv(vid_stream,8) | extend_mask[7]; /* extended sign */
      else if (l & bit_set_mask[7]) 
	l |= extend_mask[7];
    }
  else
    {
      l = r & 0xff;
      r = r >> 8;
      if (fgetb(vid_stream)) {l = -l;}
    }
  k += r;
  matrix[k++] = l;
  vid_stream->NumberNZ++;
  while(k<BLOCKSIZE)
    {
      r = Decode(vid_stream,vid_stream->huff.T1DHuff);
      if (!r) {return;} /*Eof*/
      if (r == HUFFMAN_ESCAPE)
	{
	  r = fgetv(vid_stream,6);
	  l = fgetv(vid_stream,8);
	  if (l==0x00)
	    l = fgetv(vid_stream,8); /* extended precision */
	  else if (l==0x80)
	    l = fgetv(vid_stream,8) | extend_mask[7]; /* extended sign */
	  else if (l & bit_set_mask[7]) 
	    {
	      l |= extend_mask[7];
	    }
	}
      else
	{
	  l = r & 0xff;
	  r = r >> 8;
	  if (fgetb(vid_stream))  {l = -l;}
	}

#ifdef CODEC_DEBUG
      printf("CBPDecodeAC: k:%d r:%d l:%d\n",r,l);
#endif

      if (l & bit_set_mask[7])
	l |= extend_mask[7];
      k += r;
      if (k>=BLOCKSIZE)
	{
	  WHEREAMI();
	  printf("k greater than blocksize:%d\n",k);
	  break;
	}
      matrix[k++] = l;
      vid_stream->NumberNZ++;
    }
  if ((r=Decode(vid_stream,vid_stream->huff.T1DHuff)))
    {
      WHEREAMI();
      printf("EOB expected, found 0x%x.\n",r);
    }
}

/*BFUNC

DecodeDC() decodes a dc element from the stream.

EFUNC*/

int DecodeDC(vid_stream, LocalDHuff)
     mpeg1encoder_VidStream *vid_stream;
     DHUFF *LocalDHuff;
{
  /*BEGIN("DecodeDC");*/
  int s,diff;

  s = Decode(vid_stream,LocalDHuff);
#ifdef CODEC_DEBUG
  printf("DC Decode sig. %d\n",s);
#endif

  if (s)
    {
      diff = fgetv(vid_stream,s);
      s--;                                  /* Align */

#ifdef CODEC_DEBUG
      printf("Raw DC Decode %d\n",diff);
#endif
      if ((diff & bit_set_mask[s]) == 0)
	{
	  diff |= extend_mask[s];
	  diff++;
	}
    }
  else
    diff=0;
  return(diff);
}

/*BFUNC

EncodeDC() encodes the coefficient input into the output stream.

EFUNC*/

void EncodeDC(vid_stream, coef,LocalEHuff)
     mpeg1encoder_VidStream *vid_stream;
     int coef;
     EHUFF *LocalEHuff;
{
  int cofac,s,encsize;

  if (coef) vid_stream->NumberNZ++;
  cofac = abs(coef);
  if (cofac < 256)
    s = csize[cofac];
  else
    {
      cofac = cofac >> 8;
      s = csize[cofac] + 8;
    }
#ifdef CODEC_DEBUG
  printf("DC Encoding Difference %d Size %d\n",coef,s);
#endif
  encsize=Encode(vid_stream,s,LocalEHuff);
  if (coef < 0)
    coef--;
  fputv(vid_stream,s,coef);
  vid_stream->CodedBlockBits+=(s+encsize);
}


/*END*/

