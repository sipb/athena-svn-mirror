/* 
   
  asc85ec.c
   
   This module implements an encoding filter from binary to a
   base-85 ASCII encoding (4 bytes binary -> 5 bytes ASCII).
   The ASCII encoding is a variation of that used by Rutter
   and Orost in their public-domain Unix utilities atob and btoa.
   This encoding maps 0 to the character '!' (== 0x21), and 84 to the
   character 'u' (== 0x75).  New line ('\n') characters are
   inserted so that standard ASCII tools (editors, mailers, etc.)
   can handle the resulting ASCII.  Whitespace characters,
   including '\n', will be ignored during decoding.

Original version: Ed McCreight: 1 Aug 1989
Edit History:
Ed McCreight: 8 Jan 91
End Edit History.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  This source code is provided to you by Adobe on a non-exclusive,
*  royalty-free basis to facilitate your development of PostScript
*  language programs.  You may incorporate it into your software as is
*  or modified, provided that you include the following copyright
*  notice with every copy of your software containing any portion of
*  this source code.
*
* Copyright 1990 Adobe Systems Incorporated.  All Rights Reserved.
*
* Adobe does not warrant or guarantee that this source code will
* perform in any manner.  You alone assume any risks and
* responsibilities associated with implementing, using or
* incorporating this source code into your software.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#ifndef WITHIN_PS
#define WITHIN_PS 0
#endif

#if !WITHIN_PS

#include <stdio.h>

#ifdef DOS_OS2 /* Microsoft C */
#include <io.h>		/* for setmode */
#include <fcntl.h>	/* for setmode */
#endif /* DOS_OS2 */

#include "protos.h"

#define FILTERBUFSIZE		64 /* should be a multiple of 4 */
#define BaseStm			FILE *
#define private			static
#define boolean 		int
#define true	 		1
#define os_fputc(x, y)		fputc(x, y)
#define base_ferror(stm)	ferror(stm)

typedef int		ps_size_t;

typedef struct _t_PData
{
  BaseStm baseStm;
  int charsSinceNewline;
} PDataRec, *PData;

typedef struct _t_Stm
{
  struct {
    boolean eof;
    boolean error;
  } flags;
  int cnt;
  char *ptr, base[FILTERBUFSIZE];
  PDataRec data;
} StmRec, *Stm;

#define GetPData(stm)		((PData)&((stm)->data))
#define GetBaseStm(pd)		((pd)->baseStm)

#define os_feof(stm)		((stm)->flags.eof)
#define os_ferror(stm)		((stm)->flags.error)

private int SetEOF ARGDEF1(Stm, stm)
{
  stm->flags.eof = true;
  stm->cnt = 0;
  return EOF;
}

private int SetError ARGDEF1(Stm, stm)
{
  stm->flags.error = true;
  stm->cnt = 0;
  return EOF;
}

#endif /* WITHIN_PS */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define NEWLINE_EVERY	64 /* characters */

private int Asc85EFlsBuf ARGDEF2(int, ch, register Stm, stm)
{
  int leftOver;
  register unsigned char *base;
  register PData pd = GetPData(stm);
  register BaseStm baseStm = GetBaseStm(pd);
  
  if ( base_ferror(baseStm) || os_feof(stm) || os_ferror(stm) )
    return SetError(stm);
  
  for (base = (unsigned char *)stm->base;
    base < (unsigned char *)stm->ptr - 3; base += 4)
  {
    /* convert 4 binary bytes to 5 ASCII output characters */
    register unsigned long val;
    
    if (NEWLINE_EVERY <= pd->charsSinceNewline)
    {
      if (putc('\n', baseStm) == EOF)
        return EOF;
      pd->charsSinceNewline = 0;
    }
    
    val =
      (((unsigned long) ((((unsigned int) base[0]) << 8) | base[1])) << 16) |
      ((((unsigned int) base[2]) << 8) | base[3]);
      /* The casting in this statement is carefully designed to
        preserve unsignedness and to postpone widening as
        long as possible.
      */

    if (val == 0)
    {
      putc('z', baseStm);
      pd->charsSinceNewline++;
    }
    else
    {
      /* requires 2 semi-long divides (long by short), 2
        short divides, 4 short subtracts, 5 short adds,
        4 short multiplies.  It surely would be nice if C
        had an operator that gave you quotient and remainder
        in one machine operation.
      */
      unsigned char digit[5]; /* 0..84 */
      register unsigned long q = val/7225;
      register unsigned r = (unsigned)val-7225*(unsigned)q;
      register unsigned t;
      digit[3] = (t = r/85) + '!';
      digit[4] = r - 85*t + '!';
      digit[0] = (t = q/7225) + '!';
      r = (unsigned)q - 7225*t;
      digit[1] = (t = r/85) + '!';
      digit[2] = r - 85*t + '!';
      fwrite(digit, (ps_size_t)1, (ps_size_t)5, baseStm);
      pd->charsSinceNewline += 5;
    }
  }
  
  for (leftOver = 0; base < (unsigned char *)stm->ptr; )
    stm->base[leftOver++] = *base++;
    
  stm->ptr = stm->base+leftOver;
  stm->cnt = FILTERBUFSIZE-leftOver;
  
  stm->cnt--;
  return (unsigned char)(*stm->ptr++ = (unsigned char)ch);
}

private int Asc85EPutEOF ARGDEF1(register Stm, stm)
{
  BaseStm baseStm = GetBaseStm(GetPData(stm));

  if ( os_feof(stm) )
    return EOF;
  Asc85EFlsBuf(0, stm);
  stm->ptr--;

  if (stm->base < stm->ptr)
  {
    /*  There is a remainder of from one to three binary
    bytes.  The idea is to simulate padding with 0's
    to 4 bytes, converting to 5 bytes of ASCII, and then
    sending only those high-order ASCII bytes necessary to
    express unambiguously the high-order unpadded binary.  Thus
    one byte of binary turns into two bytes of ASCII, two to three,
    and three to four.  This representation has the charm that
    it allows arbitrary-length binary files to be reproduced exactly, and
    yet the ASCII is a prefix of that produced by the Unix utility
    "btoa" (which rounds binary files upward to a multiple of four
    bytes).
    
    This code isn't optimized for speed because it is only executed at EOF.
    */
    unsigned long val = 0, power = 52200625L /* 85^4 */;
    unsigned long int i;
    for (i = 0; i < 4; i++)
      val = val << 8 |
        ((stm->base+i < stm->ptr)? (unsigned char)stm->base[i] :
        0 /* low-order padding */);
    for (i = 0; i <= (stm->ptr - stm->base); i++)
    {
      char q = val/power; /* q <= 84 */
      os_fputc(q+'!', baseStm);
      val = (val - q*power);
      power /= 85;
    }
  }
  os_fputc('~', baseStm); /* EOD marker */
  os_fputc('>', baseStm);
  SetEOF(stm);
  return (base_ferror(baseStm) || os_ferror(stm)) ? SetError(stm) : 0;
}

private int Asc85EReset ARGDEF1(register Stm, stm)
{
  stm->cnt = FILTERBUFSIZE;
  stm->ptr = stm->base;
  GetPData(stm)->charsSinceNewline = 0;
  return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if !WITHIN_PS

StmRec outStm[1];

int encode ARGDEF2(FILE *, in, FILE *, out)
{
  int bytesRead;
  GetPData(outStm)->baseStm = out;
  Asc85EReset(outStm);
  while (!feof(in))
  {
    int ch;
    while (!feof(in) & (0 < outStm->cnt))
    {
      int bytesRead =
        fread(outStm->ptr, (ps_size_t)1, (ps_size_t)outStm->cnt, in);
      outStm->ptr += bytesRead;
      outStm->cnt -= bytesRead;
    }
    ch = getc(in);
    if (ch != EOF)
    {
      if (Asc85EFlsBuf(ch, outStm) == EOF)
        return(-1);
    }
  }
  if (Asc85EPutEOF(outStm) == EOF)
    return(-1);
  return (0);
}

void main ARGDEF0()
{
  FILE *in, *out;
  int prompt = 0;
  
#ifdef SMART_C
  /* Lightspeed C for Macintosh, where there's no obvious way to
  force stdin to binary mode.
  */
  prompt = 1;
#endif /* SMART_C */

#ifdef PROMPT
  prompt = PROMPT;
#endif /* PROMPT */

  if (prompt)
  {
    char inName[100], outName[100];
    fprintf(stdout, "Input binary file: ");
    fscanf(stdin, "%s", inName);
    in = fopen(inName, "rb");
    if (in == NULL)
    {
      fprintf(stderr, "Couldn't open binary input file \"%s\"\n", inName);
      exit(1);
    }
    fprintf(stdout, "Output text file: ");
    fscanf(stdin, "%s", outName);
    out = fopen(outName, "w");
    if (out == NULL)
    {
      fprintf(stderr, "Couldn't open ASCII output file \"%s\"\n", outName);
      exit(1);
    }
  }
  else
  {
  
#ifdef DOS_OS2 /* Microsoft C */
    setmode(fileno(stdin), O_BINARY);
    setmode(fileno(stdout), O_TEXT);
#endif /* ndef DOS_OS2 */

    in = stdin;
    out = stdout;
  }
  
  if (encode(in, out))
  {
    fclose(out);
    fprintf(stderr, "\nASCII85Encode failed.\n");
    exit(1);
  }
  
  fclose(in);
  fclose(out);
  exit(0);
}

#endif /* WITHIN_PS */

