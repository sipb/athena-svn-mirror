/*
 * $Revision: 1.1.1.1 $
------------------------------------------------------------------------------
Standard definitions and types, Bob Jenkins
------------------------------------------------------------------------------
*/
#ifndef BJSTANDARD
# define BJSTANDARD
# ifndef STDLIB
#  include <stdlib.h>
#  define STDLIB
# endif
# ifndef STDIO
#  include <stdio.h>
#  define STDIO
# endif
# ifndef STDDEF
#  include <stddef.h>
#  define STDDEF
# endif
#ifdef notdef
/* These are used luckily */
typedef  unsigned long long  ub8;
typedef    signed long long  sb8;
#endif
#ifdef _LP64	/* 64-bit */
typedef  unsigned       int  ub4;   /* unsigned 4-byte quantities */
typedef    signed       int  sb4;
#else		/* 32-bit */
typedef  unsigned      long  ub4;   /* unsigned 4-byte quantities */
typedef                long  sb4;
#endif
typedef  unsigned short int  ub2;
typedef           short int  sb2;
typedef  unsigned       char ub1;
typedef                 char sb1;   /* signed 1-byte quantities */
typedef                 int  Word_t;  /* fastest type available */

#ifndef bjalign
# define bjalign(a) (((ub4)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))
#endif /* bjalign */

#define TRUE  1
#define FALSE 0
#define SUCCESS 0  /* 1 on VAX */

#endif /* BJSTANDARD */
