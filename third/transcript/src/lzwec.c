/* 
  lzwec.c

Original version: Ed McCreight: 17 Feb 90
Edit History:
Ed McCreight: 8 Mar 91
End Edit History.

    Lempel-Ziv-Welch encoding

*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  The LZW compression method is said to be the subject of patents
*  owned by the Unisys Corporation.  For further information, consult
*  the PostScript Language Reference Manual, second edition
*  (Addison Wesley, 1990, ISBN 0-201-18127-4).

*  This source code is provided to you by Adobe on a non-exclusive,
*  royalty-free basis to facilitate your development of PostScript
*  language programs.  You may incorporate it into your software as is
*  or modified, provided that you include the following copyright
*  notice with every copy of your software containing any portion of
*  this source code.
*
* Copyright 1990-91 Adobe Systems Incorporated.  All Rights Reserved.
*
* Adobe does not warrant or guarantee that this source code will
* perform in any manner.  You alone assume any risks and
* responsibilities associated with implementing, using or
* incorporating this source code into your software.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#ifndef WITHIN_PS
#define WITHIN_PS	0
#endif

#if !WITHIN_PS

#include <stdio.h>

#ifdef DOS_OS2 /* Microsoft C */
#include <io.h>		/* for setmode */
#include <fcntl.h>	/* for setmode */
#endif /* DOS_OS2 */

#include "protos.h"

#define BaseStm			FILE *
#define boolean			int
#define true			(1)
#define false			(0)
#define private			static
#define public
#define procedure		void
#define DEVELOP			1
#define STAGE			DEVELOP
#define base_ferror(stm)	ferror(stm)
#define DebugAssert(x)						\
  { if (!(x)) { fprintf(stderr, "\nCan't happen!\n"); exit(1); } }

#define readonly
typedef int		ps_size_t;

typedef struct _t_FilterData {
  BaseStm baseStm;
} FilterDataRec;

typedef struct _t_Stm
{
  struct _t_Stm_Flags
  {
    boolean eof;
    boolean error;
  } flags;
  int cnt;
  char *ptr, *base;
  void * data;
} StmRec, *Stm;

private void os_bzero ARGDEF2(char *, cp, long int, len)
{
  while (0 < len--)
    *cp++ = 0;
}

#define GetPData(stm)		(*((PData *)&((stm)->data)))
#define GetBaseStm(pd)		((pd)->fd.baseStm)

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

#include "bitstm.h"

#if !WITHIN_PS

private procedure FPutBitStm
  ARGDEF3(unsigned int, v, int, len, register BitStm, bs)
{
  PutBitStm(v, len, bs);
}

#endif /* WITHIN_PS */

#include "lzw.h"

#define HASHSIZE 8191	/* prime, approx == 2*LZWMAXCODE */

#define BUFLEN		2048 /* bytes */
#define CTOFFSET	NLITCODES

typedef struct _t_PDataRec
{
  FilterDataRec fd;
  BitStmRec bitStm[1];
  long int stmPos;
  int curCodeLen; /* 9-12 bits */
  int prevCodeWord;
  int nextAvailCodeWord;
  int limCodeWordThisLen;
  int earlyChange;
  boolean retainFullTable;
  char buf[BUFLEN];
  short int hashTab[HASHSIZE];
  LZWCodeRec zc[LZWMAXCODE];
}
PDataRec, * PData;

#if CHKSYNCH
#define Synch()							\
  FPutBitStm(SYNCHPAT, SYNCHLEN, pd->bitStm)			\
    /* insert synch pattern between code words */
#else
#define Synch()
#endif /* CHKSYNCH */

private procedure LZWEIncreaseCodeLen ARGDEF1(register PData, pd)
{
  pd->curCodeLen++;
  pd->limCodeWordThisLen = (pd->limCodeWordThisLen << 1)+
    pd->earlyChange;
}

private procedure LZWESetLimits ARGDEF1(register PData, pd)
{
  pd->curCodeLen = LZWMINCODELEN;
  pd->limCodeWordThisLen = (1 << LZWMINCODELEN)-pd->earlyChange;
  while (pd->limCodeWordThisLen < pd->nextAvailCodeWord)
  {
    if (pd->curCodeLen < LZWMAXCODELEN)
      LZWEIncreaseCodeLen(pd);
    else
      break;
  }
}

private procedure LZWEClearInit ARGDEF1(register PData, pd)
{
  pd->curCodeLen = LZWMINCODELEN;
  pd->limCodeWordThisLen = (1 << LZWMINCODELEN)-pd->earlyChange;
  pd->nextAvailCodeWord = NLITCODES;
  pd->prevCodeWord = -1;
  os_bzero((char *)pd->hashTab, (long int)(HASHSIZE*sizeof(short int)));
}

public long int nCollisions; /* = 0, for profiling */
#define NoteCollision()		nCollisions++

private int LZWEHash ARGDEF2(register PData, pd, int, ch)
{
  int hti, delta;
#if STAGE == DEVELOP
  int key1;
#endif /* STAGE == DEVELOP */
  register LZWCode zcCur;
  register int key = (((ch << 5) + pd->prevCodeWord) << 1) % HASHSIZE;
  
  DebugAssert((0 <= key) && (key < HASHSIZE));
  
  if ((hti = pd->hashTab[key]) == 0)
    return key; /* First probe hit empty entry */
  zcCur = pd->zc + hti;
  if
  (
    (zcCur->prevCodeWord == pd->prevCodeWord) &&
    (zcCur->finalChar == ch)
  )
    return key; /* First probe hit desired entry */
    
  /* First probe collided */
#if STAGE == DEVELOP
  key1 = key;
#endif /* STAGE == DEVELOP */
  delta = ch+1;
    /* Since HASHSIZE is prime, any delta EXCEPT 0 generates
      the ring of integers mod HASHSIZE.
    */
  for (;;)
  {
    NoteCollision();
    key += delta;
    if (HASHSIZE <= key)
      key -= HASHSIZE;
    DebugAssert(key != key1);
      /* hash failed -- should be impossible */

    if ((hti = pd->hashTab[key]) == 0)
      return key; /* empty entry */
    zcCur = pd->zc + hti;
    if
    (
      (zcCur->prevCodeWord == pd->prevCodeWord) &&
      (zcCur->finalChar == ch)
    )
      return key;
  }
}

private int LZWEBasicFlush ARGDEF1(register Stm, stm)
{
  register PData pd = GetPData(stm);
  char *cp;
  
  pd->bitStm->byteStm = GetBaseStm(pd);
  
  if ( base_ferror(pd->bitStm->byteStm) || os_feof(stm) || os_ferror(stm) )
    return SetError(stm);
  
  for (cp = stm->base; cp < stm->ptr; cp++)
  {
    int ch = *(unsigned char *)cp;
    
    if (pd->prevCodeWord < 0)
    {
      if ((pd->nextAvailCodeWord < NLITCODES) && (stm->base < stm->ptr))
      {
        FPutBitStm(LZW_CLEAR, pd->curCodeLen, pd->bitStm);
        Synch();
        LZWEClearInit(pd);
      }
      pd->prevCodeWord = ch;
    }
    else
    {
      int key = LZWEHash(pd, ch);
      int curCodeWord;
      if ((curCodeWord = pd->hashTab[key]) != 0)
        pd->prevCodeWord = curCodeWord;
          /* this extension is already in the table, continue... */
      else
      {
        register LZWCode zcCur;
        /* The range of possible emitted codewords
        is [0..pd->nextAvailCodeWord).  pd->curCodeLen bits must be
        sufficient to express pd->nextAvailCodeWord-1.
        */
	PutBitStm(pd->prevCodeWord, pd->curCodeLen, pd->bitStm);
	Synch();
	
        if (pd->limCodeWordThisLen <= pd->nextAvailCodeWord)
        {
          if ((LZWMAXCODE - pd->earlyChange) <= pd->nextAvailCodeWord)
          {
            /* The table is full.  Don't extend it. */
            if (!pd->retainFullTable)
            {
              /* clear the table and start over */
              FPutBitStm(ch, pd->curCodeLen, pd->bitStm);
              Synch();
              FPutBitStm(LZW_CLEAR, pd->curCodeLen, pd->bitStm);
              Synch();
              LZWEClearInit(pd);
              ch = -1;
            }
            goto EExtensionDone;
          }
          else
          {
            if (pd->curCodeLen < LZWMAXCODELEN)
              /* pd->curCodeLen bits no longer suffice to hold
              pd->nextAvailCodeWord.  Increase pd->curCodeLen.
              */
              LZWEIncreaseCodeLen(pd);
          }
	}
	
        /* Extend the table */
        zcCur = pd->zc + pd->nextAvailCodeWord;
        zcCur->prevCodeWord = pd->prevCodeWord;
        zcCur->finalChar = ch;
        pd->hashTab[key] = pd->nextAvailCodeWord;
        pd->nextAvailCodeWord++;
        
        EExtensionDone:
        pd->prevCodeWord = ch;
      }
    }
  }
  
  pd->stmPos += (stm->ptr - stm->base);
  stm->ptr = stm->base;
  stm->cnt = BUFLEN;
  return 0;
}

private int LZWEFlsBuf ARGDEF2(int, ch, register Stm, stm)
{
  if ( LZWEBasicFlush(stm) )
    return EOF;
  stm->cnt--;
  *stm->ptr++ = ch;
  return (unsigned char)ch;
}

private int LZWEReset ARGDEF1(Stm, stm)
{
  PData pd = GetPData(stm);
  stm->ptr = stm->base = pd->buf;
  stm->cnt = 0;
  pd->stmPos = 0;
  pd->bitStm->residueLen = 0;
  LZWEClearInit(pd);
  pd->nextAvailCodeWord = -1;
  return 0;
}

private int LZWEPutEOF ARGDEF1(register Stm, stm)
{
  register PData pd = GetPData(stm);
  if ( os_feof(stm) )
    return EOF;
  LZWEBasicFlush(stm);
  if (0 <= pd->prevCodeWord)
  {
    FPutBitStm(pd->prevCodeWord, pd->curCodeLen, pd->bitStm);
    Synch();
    pd->prevCodeWord = -1;
  }
  pd->nextAvailCodeWord = -1; /* force subsequent clear */
  FPutBitStm(LZW_EOD, pd->curCodeLen, pd->bitStm);
  Synch();
  while (pd->bitStm->residueLen)
    FPutBitStm(0, (- pd->bitStm->residueLen) & 0x07, pd->bitStm);
      /* finish out the last byte with 0's */
  SetEOF(stm);
  return (base_ferror(pd->bitStm->byteStm) || os_ferror(stm)) ?
    SetError(stm) : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if !WITHIN_PS

StmRec outStm[1];
PDataRec pdata[1];

private procedure PutShort ARGDEF2(short int, n, FILE *, f)
{
  putc((n >> 8) & 0xFF, f);
  putc(n & 0xFF, f);
}

private short int GetShort ARGDEF1(FILE *, f)
{
  unsigned char rhi = ((unsigned char) getc(f)) & 0xFF;
  unsigned char rlo = ((unsigned char) getc(f)) & 0xFF;
  return (short int)((rhi << 8) | rlo);
}

private boolean DoFilter ARGDEF6(
  FILE *, in,
  FILE *, out,
  int, earlyChange,
    /* 0 => TIFF spec, 1 => Aldus sample code; they're different */
  boolean, retainFullTable,
  FILE *, before,
  FILE *, after
)
{
  int bytesRead;
  PData pd;
  
  outStm->data = (void *)pdata;
  pd = GetPData(outStm);
  pd->fd.baseStm = out;
  pd->earlyChange = earlyChange;
  pd->retainFullTable = retainFullTable;
  LZWEReset(outStm);
  
  if (before != NULL)
  {
    int nacw = GetShort(before);
    pd->nextAvailCodeWord = NLITCODES;
    while (pd->nextAvailCodeWord < nacw)
    {
      int key, ch;
      register LZWCode zcCur;
      pd->prevCodeWord = GetShort(before);
      ch = getc(before);
      key = LZWEHash(pd, ch);
      
      /* Extend the table */
      zcCur = pd->zc + pd->nextAvailCodeWord;
      zcCur->prevCodeWord = pd->prevCodeWord;
      zcCur->finalChar = ch;
      pd->hashTab[key] = pd->nextAvailCodeWord;
      pd->nextAvailCodeWord++;
    }
    pd->prevCodeWord = -1;
    LZWESetLimits(pd);
    fclose(before);
  }
  
  while (!feof(in) && !ferror(in))
  {
    if (0 < outStm->cnt)
    {
      int bytesRead = fread(outStm->ptr, (ps_size_t)1,
        (ps_size_t)outStm->cnt, in);
      outStm->cnt -= bytesRead;
      outStm->ptr += bytesRead;
    }
    else
    { 
      int ch = getc(in);
      if (ch == EOF)
        break;
      LZWEFlsBuf(ch, outStm);
    }
  }
  
  LZWEBasicFlush(outStm);
  
  if (after != NULL)
  {
    int nacw;
    PutShort((short int) pd->nextAvailCodeWord, after);
    
    nacw = NLITCODES;
    while (nacw < pd->nextAvailCodeWord)
    {
      register LZWCode zcCur = pd->zc + nacw;
      PutShort((short int) zcCur->prevCodeWord, after);
      putc(zcCur->finalChar, after);
      nacw++;
    }
    fclose(after);
  }
  
  LZWEPutEOF(outStm);
  return (ferror(in));
}

void main ARGDEF2(int, argc, char * *, argv)
{
  FILE *in, *out;
  int prompt = 0;
  boolean problems;
  int earlyChange = 1;
  boolean retainFullTable = false;
  FILE *before = NULL;
  FILE *after = NULL;
  char * endp;
  int i;
  char inName[100], outName[100];
  boolean debugFiles = false;
  
  inName[0] = '\0';
  outName[0] = '\0';
  
  for (i = 1; i < argc; i++)
  {
    char * arg = argv[i];
    if (*(arg++) != '-')
      goto ExplainMe;
    switch (*(arg++))
    {
      case 's':
      case 'S':
        earlyChange = 0;
        break;
        
      case 'r':
      case 'R':
        retainFullTable = 1;
        break;
        
      case 'b':
      case 'B':
        if (!(*arg))
        {
          if (++i < argc)
            arg = argv[i];
          else
            goto ExplainMe;
        }
        before = fopen(arg, "r");
        if (before == NULL)
          goto ExplainMe;
        break;
        
      case 'a':
      case 'A':
        if (!(*arg))
        {
          if (++i < argc)
            arg = argv[i];
          else
            goto ExplainMe;
        }
        after = fopen(arg, "w");
        if (after == NULL)
          goto ExplainMe;
        break;
        
      case 'd':
      case 'D':
        debugFiles = true;
        break;
        
      default:
        goto ExplainMe;
    }
  }
  
  if (debugFiles)
  {
    strcpy(inName, "test.in");
    strcpy(outName, "test.out");
  }
  else
  {
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
      fprintf(stdout, "Input file: ");
      fscanf(stdin, "%s", inName);
      fprintf(stdout, "Output LZW compressed file: ");
      fscanf(stdin, "%s", outName);
    }
  }
  
  if (inName[0])
  {
    in = fopen(inName, "rb");
    if (in == NULL)
    {
      fprintf(stderr, "Couldn't open input file \"%s\"\n", inName);
      exit(1);
    }
  }
  else
    in = stdin;
    
  if (outName[0])
  {
    out = fopen(outName, "w");
    if (out == NULL)
    {
      fprintf(stderr, "Couldn't open output file \"%s\"\n", outName);
      exit(1);
    }
  }
  else
    out = stdout;
  

#ifdef DOS_OS2 /* Microsoft C */
    setmode(fileno(in), O_BINARY);
    setmode(fileno(out), O_BINARY);
    if (before != NULL)
      setmode(fileno(before), O_BINARY);
    if (after != NULL)
      setmode(fileno(after), O_BINARY);
#endif /* DOS_OS2 */

  DoFilter(in, out, earlyChange, retainFullTable, before, after);
  fclose(in);
  fclose(out);
  exit(0);
  
 ExplainMe:
  fprintf(stderr, "Usage: %s [-s][-r][-b <file>][-a <file>]\n", argv[0]);
  fprintf(stderr, "  where\n");
  fprintf(stderr, "         -s => stretch each code length as long as possible,\n");
  fprintf(stderr, "                 following the somewhat ambiguous TIFF 5.0 spec\n");
  fprintf(stderr, "                 (default follows Aldus source code)\n");
  fprintf(stderr, "         -r => retain coding table after it fills up\n");
  fprintf(stderr, "                 (default clears it when full)\n");
  fprintf(stderr, "  -b <file> => pre-load coding table from...\n");
  fprintf(stderr, "  -a <file> => post-dump coding table to...\n");
  fprintf(stderr, "         -d => use test.in/test.out instead of stdin/stdout\n");
  exit(1);
}

#endif /* WITHIN_PS */
