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
stream.c

This file handles all of the bit-level stream commands.

************************************************************
*/

/*LABEL stream.c */

#include <stdio.h>
#include "globals.h"

/*PRIVATE*/

const int bit_set_mask[] =
{0x00000001,0x00000002,0x00000004,0x00000008,
0x00000010,0x00000020,0x00000040,0x00000080,
0x00000100,0x00000200,0x00000400,0x00000800,
0x00001000,0x00002000,0x00004000,0x00008000,
0x00010000,0x00020000,0x00040000,0x00080000,
0x00100000,0x00200000,0x00400000,0x00800000,
0x01000000,0x02000000,0x04000000,0x08000000,
0x10000000,0x20000000,0x40000000,0x80000000};

#define mput1(vs)\
{(vs)->current_write_byte|=bit_set_mask[(vs)->write_position--];\
 if ((vs)->write_position<0) {putc((vs)->current_write_byte,(vs)->swout); (vs)->write_position=7;(vs)->current_write_byte=0;}}

#define mput0(vs)\
{(vs)->write_position--;if((vs)->write_position<0){putc((vs)->current_write_byte,(vs)->swout);(vs)->write_position=7;(vs)->current_write_byte=0;}}


/*START*/

/*BFUNC

readalign() aligns the read stream to the next byte boundary.

EFUNC*/

void readalign(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("readalign"); */

  vid_stream->current_read_byte = 0;
  vid_stream->read_position = -1;
}

/*BFUNC

mropen() opens up the stream for reading on a bit level.

EFUNC*/

void mropen(vid_stream,filename)
     mpeg1encoder_VidStream *vid_stream;
     char *filename;
{
  /* BEGIN("mropen"); */

  vid_stream->current_read_byte=0;
  vid_stream->read_position = -1;
  if ((vid_stream->srin = fopen(filename,"r")) == NULL)
    {
      fprintf(stderr,"Cannot Read Input File\n");
      exit(ERROR_INIT_FILE);
    }
}

/*BFUNC

mrclose() closes a read bit-stream.

EFUNC*/

void mrclose(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("mrclose"); */

  fclose(vid_stream->srin);
}

/*BFUNC

mwopen() opens a bit stream for writing.

EFUNC*/

void mwopen(vid_stream,filename)
     mpeg1encoder_VidStream *vid_stream;
     char *filename;
{
  BEGIN("mwopen");

  vid_stream->current_write_byte=0;
  vid_stream->write_position=7;
  if ((vid_stream->swout = fopen(filename,"w+")) == NULL)
    {
      WHEREAMI();
      printf("Cannot Open Output File\n");
      exit(ERROR_INIT_FILE);
    }
}


/*BFUNC

mwclose() closes the write bit-stream. It flushes the remaining byte
with ones, consistent with -1 returned on EOF.

EFUNC*/

void mwclose(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("mwclose"); */

  while(vid_stream->write_position!=7)
    {
      mput1(vid_stream);
    }
  fclose(vid_stream->swout);
}


/*BFUNC

zeroflush() flushes out the rest of the byte with 0's.

EFUNC*/

void zeroflush(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("zeroflush"); */

/*  printf("WP: %d\n",write_position);*/
  while (vid_stream->write_position!=7)
    {
      mput0(vid_stream);
    }
}

/*BFUNC

mputb() puts a bit to the write stream.

EFUNC*/

void mputb(vid_stream,b)
     mpeg1encoder_VidStream *vid_stream;
     int b;
{
  /* BEGIN("mputb"); */

  if (b) {mput1(vid_stream);}
  else {mput0(vid_stream);}
}

/*BFUNC

mgetb() returns a bit from the read stream.

EFUNC*/

int mgetb(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("mgetb"); */

  if (vid_stream->read_position<0)
    {
      vid_stream->current_read_byte=getc(vid_stream->srin);
      vid_stream->read_position = 7;
    }
  if (vid_stream->current_read_byte&bit_set_mask[vid_stream->read_position--]) {return(1);}
  return(0);
}

/*BFUNC

mputv() puts a n bits to the stream from byte b.

EFUNC*/

void mputv(vid_stream,n,b)
     mpeg1encoder_VidStream *vid_stream;
     int n;
     int b;
{
  /* BEGIN("mputv"); */

  while(n--)
    {
      if(b&bit_set_mask[n]) {mput1(vid_stream);}
      else {mput0(vid_stream);}
    }
}

/*BFUNC

mgetv() returns n bits read off of the read stream.

EFUNC*/

int mgetv(vid_stream,n)
     mpeg1encoder_VidStream *vid_stream;
     int n;
{
  /* BEGIN("mgetv"); */
  int b=0;

  while(n--)
    {
      b <<= 1;
      if (mgetb(vid_stream)) {b |= 1;}
    }
  return(b);
}


/*BFUNC

mwtell() returns the position in bits of the write stream.

EFUNC*/

long mwtell(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("mwtell"); */

  return((ftell(vid_stream->swout)<<3) + (7 - vid_stream->write_position));
}

/*BFUNC

mrtell() returns the position in read bits of the read stream.

EFUNC*/

long mrtell(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("mrtell"); */

  return((ftell(vid_stream->srin)<<3) - (vid_stream->read_position+1));
}

/*BFUNC

mwseek() seeks to a specific bit position on the write stream.

EFUNC*/

void mwseek(vid_stream,distance)
     mpeg1encoder_VidStream *vid_stream;
     long distance;
{
  /* BEGIN("mwseek"); */
  int Length;

  if (vid_stream->write_position != 7) {putc(vid_stream->current_write_byte,vid_stream->swout);}
  fseek(vid_stream->swout,0,2L);
  Length = ftell(vid_stream->swout);
  fseek(vid_stream->swout,(distance+7)>>3,0L);
  if ((Length << 3) <= distance) /* Make sure we read clean stuff */
    {
      vid_stream->current_write_byte = 0;
      vid_stream->write_position = 7 - (distance & 0x7);
    }
  else
    {
      vid_stream->current_write_byte = getc(vid_stream->swout);
      vid_stream->write_position = 7 - (distance & 0x7);
      fseek(vid_stream->swout,(distance+7)>>3,0L);
    }
}


/*BFUNC

mrseek() seeks to a specific bit position on the read stream.

EFUNC*/

void mrseek(vid_stream,distance)
     mpeg1encoder_VidStream *vid_stream;
     long distance;
{
  /* BEGIN("mrseek"); */

  fseek(vid_stream->srin,distance>>3,0L);
  vid_stream->current_read_byte = getc(vid_stream->srin);
  vid_stream->read_position = 7 - (distance % 8);
}


/*BFUNC

seof() returns a -1 if at the end of stream. It mimics the {\tt feof}
command.

EFUNC*/

int seof(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("seof"); */

  return(feof(vid_stream->srin));
}

/*END*/

