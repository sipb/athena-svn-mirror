/* 
  bitstm.h
    Bit sequence operators layered on Stm's
 
Original version: Ed McCreight: 18 Feb 1990
Edit History:
Ed McCreight: 27 Nov 90
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

#ifndef BITSTM_H
#define BITSTM_H

typedef struct _t_BitStmRec
{
  BaseStm byteStm;
  unsigned long residue;
    /* low-order aligned */
  int residueLen;
  boolean get24BitsIsSafe;
    /* whether it is safe to add 24 bits to residue.
      This is an implicit promise that these bits will eventually
      be needed (in case of filters), and that no
      bits that are currently outstanding will later be
      returned to residue.
    */
}
BitStmRec, * BitStm;

#define FlsBitStm(bs)							\
  while (8 <= bs->residueLen)						\
  {									\
    bs->residueLen -= 8;						\
    putc((((unsigned char)(bs->residue >> bs->residueLen)) & 0xFF),	\
      bs->byteStm);							\
  }

#define PutBitStm(v, len, bs)						\
  FlsBitStm(bs);							\
  bs->residue = (bs->residue << len) | v;				\
  bs->residueLen += len;

#define FilBitStm(bs, len)						\
  while (bs->residueLen < len)						\
  {									\
    bs->residue <<= 8;							\
    if (0 < bs->byteStm->cnt--)						\
      bs->residue |= (*((unsigned char *)bs->byteStm->ptr++));		\
    else								\
    {									\
      int ch;								\
      if ((ch = (*bs->byteStm->procs->FilBuf)(bs->byteStm)) == EOF)	\
        goto ByteStmEnd;						\
      bs->residue |= (unsigned char)ch;					\
    }									\
    bs->residueLen += 8;						\
  }

#define GetBitStm(v, len, mask, bs)						\
  FilBitStm(bs, len);							\
  v = (bs->residue >> (bs->residueLen -= len)) & mask;


extern int FGetBitStm ARGDECL2(int, len, BitStm, bs);

extern procedure FPutBitStm ARGDECL3(unsigned int, v, int, len, BitStm, bs);

#endif /* BITSTM_H */
