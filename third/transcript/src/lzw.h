/* 
  lzw.h
 
Original version: Ed McCreight: 19 Feb 90
Edit History:
Ed McCreight: 23 Feb 90
End Edit History.

    Lempel-Ziv-Welch filters
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  The LZW compression method is said to be the subject of patents
*  owned by the Unisys Corporation.  For further information, consult
*  the PostScript Language Reference Manual, second edition
*  (Addison Wesley, 1990, ISBN 0-201-18127-4).
*
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

#ifndef LZW_H

#define LZW_H

#ifndef CHKSYNCH
#define CHKSYNCH	0	/* for debugging */
#endif /* CHKSYNCH */
#define SYNCHPAT	0x0C
#define SYNCHLEN	4

#define LZWMAXCODE	4096 /* codes */
#define LZWMINCODELEN	9 /* bits */
#define LZWMAXCODELEN	12 /* bits */

/* This LZW coding is intended to be identical to the TIFF 5.0 spec.
   
   Codes 0 - 255 represent their literal byte values
   Code 256 is the "Clear" code
   Code 257 is the "EOD" code
   Codes >=258 represent multi-byte sequences
*/
#define LZW_CLEAR	256
#define LZW_EOD		257
#define NLITCODES	258

typedef struct _t_LZWCodeRec
{
  unsigned short prevCodeWord;
  unsigned char finalChar;
  unsigned char seqLen;
    /* 0 means undefined,
       1 means special code (LZW_CLEAR, LZW_EOD),
       2 means length 1,
       3 means length 2,
       ...,
       255 means length 254 or longer
    */
}
LZWCodeRec, * LZWCode;

#endif /* LZW_H */

