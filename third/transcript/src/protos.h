/*
  protos.h
  
Original version: Ed McCreight: 30 Oct 90
Edit History:
Ed McCreight: 30 Oct 90
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

#ifndef	PROTOS_H
#define	PROTOS_H

#ifndef PROTOTYPES
#ifdef __STDC__
#define PROTOTYPES	__STDC__
#else /* __STDC__ */
#define PROTOTYPES	0
#endif /* __STDC__ */
#endif /* PROTOTYPES */

/*
 * PROTOTYPES indicates whether or not ANSI function prototypes
 * are to be generated (or else ignored as comments) in compiling the
 * product.  Such function prototype macros should look something like this:
 */
 
#if 0 /* * *   B e g i n   c o m m e n t   * * */

/* ... in foozle.h: */

extern procedure foo ARGDECL2(integer, arg1, char *, arg2);
  /* if PROTOTYPES, expands to
  
        extern procedure foo (integer arg1, char * arg2);
        
     if not, expands to
  
        extern procedure foo ();
  */
    
		
/* ... in foozle.c: */

public procedure foo ARGDEF2(integer, arg1, char *, arg2)
  /* if PROTOTYPES, expands to
  
        public procedure foo (integer arg1, char * arg2)
        
     if not, expands to
  
        public procedure foo (arg1, arg2)
        integer arg1;
        char * arg2;
  */
{
  ...
}
		
#endif /* * *   E n d   c o m m e n t   * * */

/*
 * In a non-ANSI system prototypes are good documentation.
 * They also help ANSI compilers catch type-mismatch errors.
 *
 * Portability warning:  Prototypes alter the rules for function argument
 * type conversions.  Be sure that all references
 * to prototype-defined functions are in the scope of prototype
 * declarations, and that any function that is prototype-declared
 * is also prototype-defined.  The latter can be checked by the compiler,
 * but the former generally cannot.  Since a prototyped definition
 * serve as a prototyped declaration, a private function does not
 * need a separate declaration if its definition textually precedes
 * its first use.  This definition-before-use order is natural to
 * Pascal programmers.
 */

#if PROTOTYPES

#define ARGDEF0()							\
  (void)
#define ARGDEF1(t1,v1)							\
  (t1 v1)
#define ARGDEF2(t1,v1,t2,v2)						\
  (t1 v1,t2 v2)
#define ARGDEF3(t1,v1,t2,v2,t3,v3)					\
  (t1 v1,t2 v2,t3 v3)
#define ARGDEF4(t1,v1,t2,v2,t3,v3,t4,v4)				\
  (t1 v1,t2 v2,t3 v3,t4 v4)
#define ARGDEF5(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5)				\
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5)
#define ARGDEF6(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6)			\
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6)
#define ARGDEF7(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7)		\
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7)
#define ARGDEF8(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8)	\
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8)
#define ARGDEF9(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9)	\
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9)
#define ARGDEF10(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9,t10,v10) \
  (t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10)
  
#define ARGDEFV1(t1,v1)							\
  (t1 v1, ...)
#define ARGDEFV2(t1,v1,t2,v2)						\
  (t1 v1,t2 v2, ...)
#define ARGDEFV3(t1,v1,t2,v2,t3,v3)					\
  (t1 v1,t2 v2,t3 v3, ...)

#define ARGDECL0	ARGDEF0
#define ARGDECL1	ARGDEF1
#define ARGDECL2	ARGDEF2
#define ARGDECL3	ARGDEF3
#define ARGDECL4	ARGDEF4
#define ARGDECL5	ARGDEF5
#define ARGDECL6	ARGDEF6
#define ARGDECL7	ARGDEF7
#define ARGDECL8	ARGDEF8
#define ARGDECL9	ARGDEF9

#define ARGDECLV1	ARGDEFV1
#define ARGDECLV2	ARGDEFV2
#define ARGDECLV3	ARGDEFV3

#else /* PROTOTYPES */

#define ARGDEF0()							\
  ()
#define ARGDEF1(t1,v1)							\
  (v1) t1 v1;
#define ARGDEF2(t1,v1,t2,v2)						\
  (v1,v2) t1 v1; t2 v2;
#define ARGDEF3(t1,v1,t2,v2,t3,v3)					\
  (v1,v2,v3) t1 v1; t2 v2; t3 v3;
#define ARGDEF4(t1,v1,t2,v2,t3,v3,t4,v4)				\
  (v1,v2,v3,v4) t1 v1; t2 v2; t3 v3; t4 v4;
#define ARGDEF5(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5)				\
  (v1,v2,v3,v4,v5) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5;
#define ARGDEF6(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6)			\
  (v1,v2,v3,v4,v5,v6) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6;
#define ARGDEF7(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7)		\
  (v1,v2,v3,v4,v5,v6,v7)						\
  t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7;
#define ARGDEF8(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8)	\
  (v1,v2,v3,v4,v5,v6,v7,v8)						\
  t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7; t8 v8;
#define ARGDEF9(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9)	\
  (v1,v2,v3,v4,v5,v6,v7,v8,v9)						\
  t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7; t8 v8; t9 v9;
#define ARGDEF10(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9,t10,v10) \
  (v1,v2,v3,v4,v5,v6,v7,v8,v9,v10)					\
  t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7; t8 v8; t9 v9; t10 v10;


#define ARGDEFV1(t1,v1)							\
  (v1, ...) t1 v1;
#define ARGDEFV2(t1,v1,t2,v2)						\
  (v1,v2, ...) t1 v1; t2 v2;
#define ARGDEFV3(t1,v1,t2,v2,t3,v3)					\
  (v1,v2,v3, ...) t1 v1; t2 v2; t3 v3;

#define ARGDECL0()							\
  ()
#define ARGDECL1(t1,v1)							\
  ()
#define ARGDECL2(t1,v1,t2,v2)						\
  ()
#define ARGDECL3(t1,v1,t2,v2,t3,v3)					\
  ()
#define ARGDECL4(t1,v1,t2,v2,t3,v3,t4,v4)				\
  ()
#define ARGDECL5(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5)				\
  ()
#define ARGDECL6(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6)			\
  ()
#define ARGDECL7(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7)		\
  ()
#define ARGDECL8(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8)	\
  ()
#define ARGDECL9(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9)	\
  ()
#define ARGDECL10(t1,v1,t2,v2,t3,v3,t4,v4,t5,v5,t6,v6,t7,v7,t8,v8,t9,v9,t10,v10) \
  ()

#define ARGDECLV1(t1,v1)						\
  ()
#define ARGDECLV2(t1,v1,t2,v2)						\
  ()
#define ARGDECLV3(t1,v1,t2,v2,t3,v3)					\
  ()

#endif /* PROTOTYPES */

#endif	/* PROTOS_H */
