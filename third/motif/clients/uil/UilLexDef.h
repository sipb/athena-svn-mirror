/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.4
*/ 
/*   $RCSfile: UilLexDef.h,v $ $Revision: 1.1.1.1 $ $Date: 1999-01-30 03:16:38 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the UIL lexical
**	analyzer.
**
**--
**/

#ifndef UilLexDef_h
#define UilLexDef_h


/*
**  Define flags to indicate whether certain characters are to be
**  filtered in text output.
*/

#define		lex_m_filter_tab	(1 << 0)

/*
**  Define the default character set.  In Motif, the default character set is
**  not isolatin1, but simply the null string, thus we must be able to
**  distinguish the two.
*/


#define lex_k_default_charset	    -1
#define lex_k_userdefined_charset   -2
#define lex_k_fontlist_default_tag  -3

/*
**  Since key_k_keyword_max_length assumes the length of the longest
**  WML generated keyword, we need a new constant to define the
**  longest allowable identifier.  This length should not exceed
**  URMMaxIndexLen.  (CR 5566)
*/

#define lex_k_identifier_max_length	31

#endif /* UilLexDef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
