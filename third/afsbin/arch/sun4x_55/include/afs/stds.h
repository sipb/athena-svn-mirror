/* Copyright (C) 1990 Transarc Corporation - All rights reserved */

/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/config/RCS/stds.h,v 2.24 1996/09/26 17:20:00 zumach Exp $ */

#ifndef TRANSARC_AFS_CONFIG_STDS_H
#define TRANSARC_AFS_CONFIG_STDS_H	1

#include <afs/param.h>
#include <sys/types.h>

/* The following two groups are from /afs/tr/kansas/src/stds/transarc_stds.h
 * entered by Erik Brown.  900301 -ota */

#define IN              /* indicates a parameter is read in */
#define OUT             /* indicates a parameter is sent out (a ptr) */
#define INOUT           /* indicates a parameter is read in and sent out (a ptr) */
#define EXPORT          /* available to everyone */
#define HIDDEN		/* used in a PUBLIC macro, but not exported */
#define SHARED          /* shared between module source files */ 
#define PRIVATE static  /* private to a source (.c) file */
#define IMPORT extern	/* this is new -ota */

#ifndef	MACRO_BEGIN
#define MACRO_BEGIN	do {
#endif
#ifndef	MACRO_END
#define MACRO_END	} while (0)
#endif

/*
 * The following are provided for ANSI C compatibility
 */
#ifdef __STDC__
typedef void *opaque;
#else /* __STDC__ */
#define const
typedef char *opaque;
#endif /* __STDC__ */

/* Use _TAKES with double parentheses around an ANSI C param list */
/* MIPS compilers understand ANSI prototypes (and check 'em too!) */

#ifndef _TAKES
#if defined(__STDC__) || defined(mips)
#define _TAKES(x) x
#else /* __STDC__ */
#define _TAKES(x) ()
#endif /* __STDC__ */
#endif /* _TAKES */

#ifndef _ATT4
#if defined(__HIGHC__)
/*
 * keep HC from complaining about the use of "old-style" function definitions
 * with prototypes
 */
pragma Off(Prototype_override_warnings);
#endif /* defined(__HIGHC__) */
#endif
/*
 * This makes including the RCS id in object files less painful.  Put this near
 * the beginning of .c files (not .h files).  Do NOT follow it with a
 * semi-colon.  The argument should be a double quoted string containing the
 * standard RCS Header keyword.
 */

#ifndef lint

/* Believe it or not this seems to supress all warnings from all compilers I've
 * checked.  I've tried the HC compile with normal, -g and -o switched.  Also
 * gcc on the pmax with "-O -Wall" and so far so good. -ota 900426 */

#define RCSID(x) \
  static const char *rcsid = x; \
  static int (*__rcsid)(); \
  static int _rcsid () \
  { static int a; \
    a = (int)rcsid; __rcsid = _rcsid; a = (int)__rcsid; return a; }

/* #define RCSID(x) static const char *rcsid() { return x;} */

#else
#define RCSID(x)
#endif

/* Now some types to enhance portability.  Always use these on the wire or when
 * laying out shared structures on disk. */

/* Imagine that this worked...
#if (sizeof(long) != 4) || (sizeof(short) != 2)
#error We require size of long and pointers to be equal
#endif */

typedef short	         int16;
typedef unsigned short u_int16;
#ifdef	AFS_64BIT_ENV
typedef int	         int32;
#if defined(AFS_SGI53_ENV)
/* While it's a warning for a 64 bit compile, it's an error for a 32 bit
 * compile if the following is re-typedef'd...... And we need to be able to
 * #undef it if required.
 */
#define u_int32 unsigned int;
#else
typedef unsigned int  u_int32;
#endif
#else	/* AFS_64BIT_ENV */
typedef long	         int32;
#endif	/* AFS_64BIT_ENV */

/* The Sun RPC include files define this with a typedef and this caused
 * problems for the NFS Translator.  Users of those include files should just
 * #undef this. */
#if	!defined(AFS_OSF_ENV)
#define u_int32 unsigned int
#define uint32 unsigned int
/* typedef unsigned int	u_int32; */
#endif

#if !defined(AFS_ALPHA_ENV) && !defined(AFS_SGI61_ENV)
#define	xdr_int32	xdr_long
#define	xdr_u_int32	xdr_u_long
#endif

/* you still have to include <netinet/in.h> to make these work */

#define hton32 htonl
#define hton16 htons
#define ntoh32 ntohl
#define ntoh16 ntohs


/* Since there is going to be considerable use of 64 bit integers we provide
 * some assistence in this matter.  The hyper type is supposed to be compatible
 * with the afsHyper type: the same macros will work on both. */

#if	defined(AFS_64BIT_ENV) && 0

typedef	unsigned long	hyper;

#define	hcmp(a,b)	((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))
#define	hsame(a,b)	((a) == (b))
#define	hiszero(a)	((a) == 0)
#define	hfitsin32(a)	((a) & 0xffffffff00000000) == 0)
#define	hset(a,b)	((a) = (b))
#define	hzero(a)	((a) = 0)
#define	hones(a)	((a) = ~((unsigned long)0))
#define	hget32(i,a)	((i) = (unsigned int)(a))
#define	hget64(hi,lo,a)	((lo) = ((unsigned int)(a)), (hi) = ((a) & (0xffffffff00000000)))
#define	hset32(a,i)	((a) = ((unsigned int)(i)))
#define	hset64(a,hi,lo)	((a) = ((hi) | (lo)))
#define hgetlo(a)	((a) & 0xffffffff)
#define hgethi(a)	(((unsigned int)(a)) >> 32)
#define	hadd(a,b)	((a) += (b))
/* XXX */
#define	hadd32(a,b)	((a) += (b))

#else	/* AFS_64BIT_ENV */

typedef struct hyper { /* unsigned 64 bit integers */
    unsigned int high;
    unsigned int low;
} hyper;

#define hcmp(a,b) ((a).high<(b).high? -1 : ((a).high > (b).high? 1 : \
    ((a).low <(b).low? -1 : ((a).low > (b).low? 1 : 0))))
#define hsame(a,b) ((a).low == (b).low && (a).high == (b).high)
#define hiszero(a) ((a).low == 0 && (a).high == 0)
#define hfitsin32(a) ((a).high == 0)
#define hset(a,b) ((a) = (b))
#define hzero(a) ((a).low = 0, (a).high = 0)
#define hones(a) ((a).low = 0xffffffff, (a).high = 0xffffffff)
#define hget32(i,a) ((i) = (a).low)
#define hget64(hi,lo,a) ((lo) = (a).low, (hi) = (a).high)
#define hset32(a,i) ((a).high = 0, (a).low = (i))
#define hset64(a,hi,lo) ((a).high = (hi), (a).low = (lo))
#define hgetlo(a) ((a).low)
#define hgethi(a) ((a).high)

/* The algorithm here is to check for two cases that cause a carry.  If the top
 * two bits are different then if the sum has the top bit off then there must
 * have been a carry.  If the top bits are both one then there is always a
 * carry.  We assume 32 bit ints and twos complement arithmetic. */

#define SIGN 0x80000000
#define hadd32(a,i) \
    (((((a).low ^ (int)(i)) & SIGN) \
      ? (((((a).low + (int)(i)) & SIGN) == 0) && (a).high++) \
      : (((a).low & (int)(i) & SIGN) && (a).high++)), \
     (a).low += (int)(i))

#define hadd(a,b) (hadd32(a,(b).low), (a).high += (b).high)
#endif	/* AFS_64BIT_ENV */

#ifndef	KERNEL
#define max(a, b)               ((a) < (b) ? (b) : (a))
#define min(a, b)               ((a) > (b) ? (b) : (a))
/*#define abs(x)                  ((x) >= 0 ? (x) : -(x))*/
#endif
#endif /* TRANSARC_CONFIG_AFS_STDS_H */
