/* Copyright (C) 1997 by the Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <mit-copyright.h>
#include "config.h"

/* This file provides *just* enough code to implement a vsnprintf clone.
 * Massively modified from BSD 4.4lite source.
 * Non-MIT copyrights are included below at the places where they apply.
 */

static char real_rcsid[] = "$Id: my_vsnprintf.c,v 1.1 1999-03-06 16:48:17 ghudson Exp $";

/** definitions that you may want to tweak if you want/don't want features **/

/* Define this if you want to use floating-point formats (%[eEfgG]). */
#undef VSNPRINTF_FLOATING_POINT
/* NOTE: The floating point code depends on a messy __dtoa implementation
 *       (at the bottom of this file), which may not DTRT on all platforms
 *       without tweaking the constants there for things such as byte
 *       order.
 */

/* Define this if you want '%m' to interpolate the current error message. */
#define VSNPRINTF_ERRMSG_CONV

/* The following symbols are used, and should be set via Autoconf or
 * otherwise.  These symbols are used generally:
 *   HAVE_COM_ERR    -lcom_err and <com_err.h> are available
 *   HAVE_STRERROR   strerror() is available
 * These are used to enable extra-long integer formats:
 *   HAVE_QUAD_T     this platform defines quad_t and u_quad_t integer types
 *   HAVE_LONG_LONG  this platform has long long and unsigned long long types
 * These are used only when VSNPRINTF_FLOATING_POINT is defined:
 *   HAVE_ISINF      isinf() is available
 *   HAVE_FINITE     finite() is available
 *   HAVE_IEEEFP_H   header file <ieeefp.h> is available
 */

/** various includes **/

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#ifdef   VSNPRINTF_FLOATING_POINT
#include   <math.h>
#include   <float.h>
#ifdef     HAVE_IEEEFP_H
#include     <ieeefp.h>
#endif     /* HAVE_IEEEFP_H */
#endif   /* VSNPRINTF_FLOATING_POINT */

#if      defined(VSNPRINTF_ERRMSG_CONV) && defined(HAVE_COM_ERR)
#include <com_err.h>
#endif   /* VSNPRINTF_ERRMSG_CONV and HAVE_COM_ERR */

/** further defines that should just work, but may not **/

/* On systems that need/want it, map "long long" onto "quad_t" and
 * "unsigned long long" onto "u_quad_t"
 */
#if !defined(HAVE_QUAD_T) && defined(HAVE_LONG_LONG)
typedef unsigned long long   u_quad_t;
typedef long long            quad_t;

#define UQUAD_MAX  ULLONG_MAX
#define QUAD_MAX   LLONG_MAX
#define QUAD_MIN   LLONG_MIN

#define HAVE_QUAD_T
#endif /* not HAVE_QUAD_T and yes HAVE_LONG_LONG */

/* On systems that don't have isinf() but do have finite(), define one
 * in terms of the other.
 */
#if defined(VSNPRINTF_FLOATING_POINT) && !defined(HAVE_ISINF) && defined(HAVE_FINITE)
#define isinf(x)  (! finite((x)))
#endif /* VSNPRINTF_FLOATING_POINT and don't HAVE_ISINF and HAVE_FINITE */

/* For error messages: if HAVE_COM_ERR is defined, just use
 * error_message from com_err; otherwise define it as strerror().  On
 * systems that don't have strerror(), emulate using sys_errlist.
 */
#if     defined(VSNPRINTF_ERRMSG_CONV) && !defined(HAVE_COM_ERR)
#ifndef   HAVE_STRERROR
extern      const char *const sys_errlist[];
#define     strerror(x)   (sys_errlist[(x)])
#endif    /* don't HAVE_STRERROR */

#define   error_message(x)   strerror(x)
#endif  /* VSNPRINTF_ERRMSG_CONV and don't HAVE_COM_ERR */

/** it's actual code, but it seems pretty evil to me **/ 

int my_vsnprintf(char *str, size_t n, const char *fmt0, va_list ap);

/* Write formatted data into a string.
 * Arguments: str -- destination string
 *            n --   maximum # bytes to be written (including the trailing NUL)
 *            fmt -- format, like the one used by printf(3)
 *            ... -- arguments to be inserted as per the format
 * Returns: number of characters that would be written if n were infinite.
 * Note: if VSNPRINTF_ERRMSG_CONV is defined, then the '%m' conversion
 *       specifier is supported.  It writes out the error message
 *       corresponding to the current errno, and doesn't take any arguments
 *       from the arguments list.
 */
int my_snprintf(char *str, size_t n, const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = my_vsnprintf(str, n, fmt, ap);
  va_end(ap);
  return ret;
}

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*** 4.4lite/usr/src/include/stdio.h ***/

/* stdio buffers */
/*
 * stdio state variables.
 */
typedef	struct __sSTRING {
	unsigned char *pos;	/* current position in the buffer [was _p] */
	int	 width;		/* write space left [was _w] */
} STRING;

/*** 4.4lite/usr/src/lib/libc/stdio/fvwrite.h ***/

/*
 * I/O descriptors for __sfvwrite().
 */
struct __siov {
	void	*iov_base;
	size_t	iov_len;
};
struct __suio {
	struct	__siov *uio_iov;
	int	uio_iovcnt;
	int	uio_resid;
};

/*** 4.4lite/usr/src/lib/libc/stdio/floatio.h ***/

/*
 * Floating point scanf/printf (input/output) definitions.
 */

/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define	MAXEXP		308
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define	MAXFRACT	39


/*** 4.4lite/usr/src/lib/libc/stdio/fvwrite.c ***/

static char sccsid_fvwrite[] = "was: @(#)fvwrite.c	8.1 (Berkeley) 6/4/93";

/*
 * Write some memory regions.  Return zero on success, -1 on error.
 *
 * This routine is large and unsightly, but most of the ugliness due
 * to the three different kinds of output buffering is handled here.
 */
static int __sfvwrite(STRING *sp, struct __suio *uio)
{
	size_t len;
	char *p;
	struct __siov *iov;
	int w;

	if ((len = uio->uio_resid) == 0)
		return (0);

	iov = uio->uio_iov;
	p = iov->iov_base;
	len = iov->iov_len;
	iov++;
	/*
	 * String output is a special case: write as many bytes
	 * as fit, but pretend we wrote everything.  This makes
	 * snprintf() return the number of bytes needed, rather
	 * than the number used, and avoids its write function
	 * (so that the write function can be invalid).
	 */
	do {
		while (len == 0) {
			p = iov->iov_base;
			len = iov->iov_len;
			iov++;
		}
		w = sp->width;

		if (len < w)
			w = len;
		memcpy(sp->pos, p, (size_t)w);
						/* copy MIN(sp->width,len), */
		sp->width -= w;
		sp->pos += w;
		w = len;	/* but pretend copied all */

		p += w;
		len -= w;
	} while ((uio->uio_resid -= w) != 0);
	return (0);
}


/*** 4.4lite/usr/src/lib/libc/stdio/vsnprintf.c ***/

static char sccsid_vsnprintf[] = "was: @(#)vsnprintf.c	8.1 (Berkeley) 6/4/93";

/*** 4.4lite/usr/src/lib/libc/stdio/vfprintf.c ***/

static char sccsid_vfprintf[] = "was: @(#)vfprintf.c	8.1 (Berkeley) 6/4/93";

/*
 * Actual printf innards.
 *
 * This code is large and complicated...
 */

/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
static int __sprint(STRING *sp, struct __suio *uio)
{
	int err;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	err = __sfvwrite(sp, uio);
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Convert an unsigned long to ASCII for printf purposes, returning
 * a pointer to the first character of the string representation.
 * Octal numbers can be forced to have a leading zero; hex numbers
 * use the given digits.
 */
static char *__ultoa(u_long val, char *endp, int base,
		     int octzero, char *xdigs)
{
	char *cp = endp;
	long sval;

	/*
	 * Handle the three cases separately, in the hope of getting
	 * better/faster code.
	 */
	switch (base) {
	case 10:
		if (val < 10) {	/* many numbers are 1 digit */
			*--cp = to_char(val);
			return (cp);
		}
		/*
		 * On many machines, unsigned arithmetic is harder than
		 * signed arithmetic, so we do at most one unsigned mod and
		 * divide; this is sufficient to reduce the range of
		 * the incoming value to where signed arithmetic works.
		 */
		if (val > LONG_MAX) {
			*--cp = to_char(val % 10);
			sval = val / 10;
		} else
			sval = val;
		do {
			*--cp = to_char(sval % 10);
			sval /= 10;
		} while (sval != 0);
		break;

	case 8:
		do {
			*--cp = to_char(val & 7);
			val >>= 3;
		} while (val);
		if (octzero && *cp != '0')
			*--cp = '0';
		break;

	case 16:
		do {
			*--cp = xdigs[val & 15];
			val >>= 4;
		} while (val);
		break;

	default:			/* oops */
		abort();
	}
	return (cp);
}

#ifdef HAVE_QUAD_T
/* Identical to __ultoa, but for quads. */
static char *__uqtoa(u_quad_t val, char *endp, int base,
		     int octzero, char *xdigs)
{
	char *cp = endp;
	quad_t sval;

	/* quick test for small values; __ultoa is typically much faster */
	/* (perhaps instead we should run until small, then call __ultoa?) */
	if (val <= ULONG_MAX)
		return (__ultoa((u_long)val, endp, base, octzero, xdigs));
	switch (base) {
	case 10:
		if (val < 10) {
			*--cp = to_char(val % 10);
			return (cp);
		}
		if (val > QUAD_MAX) {
			*--cp = to_char(val % 10);
			sval = val / 10;
		} else
			sval = val;
		do {
			*--cp = to_char(sval % 10);
			sval /= 10;
		} while (sval != 0);
		break;

	case 8:
		do {
			*--cp = to_char(val & 7);
			val >>= 3;
		} while (val);
		if (octzero && *cp != '0')
			*--cp = '0';
		break;

	case 16:
		do {
			*--cp = xdigs[val & 15];
			val >>= 4;
		} while (val);
		break;

	default:
		abort();
	}
	return (cp);
}
#endif /* HAVE_QUAD_T */

#ifdef VSNPRINTF_FLOATING_POINT

#define	BUF		(MAXEXP+MAXFRACT+1)	/* + decimal point */
#define	DEFPREC		6

static char *cvt (double, int, int, char *, int *, int, int *);
static int exponent (char *, int, int);

#else /* no VSNPRINTF_FLOATING_POINT */

#define	BUF		68

#endif /* VSNPRINTF_FLOATING_POINT */


/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double; unimplemented */
#define	LONGINT		0x010		/* long integer */
#define	QUADINT		0x020		/* quad integer */
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */

/* Write formatted data from a va_alist into a string.
 * Arguments: str -- destination string
 *            n --   maximum # bytes to be written (including the trailing NUL)
 *            fmt -- format, like the one used by printf(3)
 *            ap --  a va_start()'ed varargs list of arguments to be inserted
 * Returns: number of characters that would be written if n were infinite.
 * Note: if VSNPRINTF_ERRMSG_CONV is defined, then the '%m' conversion
 *       specifier is supported.  It writes out the error message
 *       corresponding to the current errno, and doesn't take any arguments
 *       from the arguments list.
 */
int my_vsnprintf(char *str, size_t strsize, const char *fmt0, va_list ap)
{
	STRING f;		/* structure containing buffering data */

	char *fmt;		/* format string */
	int ch;			/* character from fmt */
	int n;			/* handy integer (short term usage) */
	char *cp;		/* handy char pointer (short term usage) */
	struct __siov *iovp;	/* for PRINT macro */
	int flags;		/* flags as above */
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef VSNPRINTF_FLOATING_POINT
	char softsign;		/* temporary negative sign for floats */
	double _double;		/* double precision arguments %[eEfgG] */
	int expt;		/* integer value of exponent */
	int expsize;		/* character count for expstr */
	int ndig;		/* actual number of digits returned by cvt */
	char expstr[7];		/* buffer for exponent string */
#endif
	u_long	ulval;		/* integer arguments %[diouxX] */
#ifdef HAVE_QUAD_T
	u_quad_t uqval;		/* %q integers */
#endif /* HAVE_QUAD_T */
	int base;		/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int fieldsz;		/* field size expanded by sign, etc */
	int realsz;		/* field size expanded by dprec */
	int size;		/* size of converted field or string */
	char *xdigs;		/* digits for [xX] conversion */
#define NIOV 8
	struct __suio uio;	/* output information: summary */
	struct __siov iov[NIOV];/* ... and individual io vectors */
	char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	char ox[2];		/* space for 0x hex-prefix */

	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static char blanks[PADSIZE] =
	 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static char zeroes[PADSIZE] =
	 {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#define	PRINT(ptr, len) { \
	iovp->iov_base = (ptr); \
	iovp->iov_len = (len); \
	uio.uio_resid += (len); \
	iovp++; \
	if (++uio.uio_iovcnt >= NIOV) { \
		if (__sprint(&f, &uio)) \
			return ret; \
		iovp = iov; \
	} \
}
#define	PAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			PRINT(with, PADSIZE); \
			n -= PADSIZE; \
		} \
		PRINT(with, n); \
	} \
}
#define	FLUSH() { \
	if (uio.uio_resid && __sprint(&f, &uio)) \
		return ret; \
	uio.uio_iovcnt = 0; \
	iovp = iov; \
}

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#define	SARG() \
	(flags&LONGINT ? va_arg(ap, long) : \
	    flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
	    (long)va_arg(ap, int))
#define	UARG() \
	(flags&LONGINT ? va_arg(ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)va_arg(ap, int) : \
	    (u_long)va_arg(ap, u_int))


	if ((int)strsize < 1)
		return (-1);
	f.pos = (unsigned char *)str;
	f.width = strsize - 1;

	fmt = (char *)fmt0;
	uio.uio_iov = iovp = iov;
	uio.uio_resid = 0;
	uio.uio_iovcnt = 0;
	ret = 0;

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
			/* void */;
		if ((n = fmt - cp) != 0) {
			PRINT(cp, n);
			ret += n;
		}
		if (ch == '\0')
			goto done;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = '\0';

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			if ((width = va_arg(ap, int)) >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
				n = va_arg(ap, int);
				prec = n < 0 ? -1 : n;
				goto rflag;
			}
			n = 0;
			while (is_digit(ch)) {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			} while (is_digit(ch));
			width = n;
			goto reswitch;
#ifdef VSNPRINTF_FLOATING_POINT
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
#if defined(HAVE_QUAD_T) && defined(HAVE_LONG_LONG)
			/* Solaris uses "%lld" for long long values */
			if (flags & LONGINT)
			  flags = (flags & ~LONGINT) | QUADINT;
			else
#endif /* HAVE_QUAD_T and HAVE_LONG_LONG */
			  flags |= LONGINT;
			goto rflag;
#ifdef HAVE_QUAD_T
		case 'q':
			flags |= QUADINT;
			goto rflag;
#endif /* HAVE_QUAD_T */
		case 'c':
			*(cp = buf) = va_arg(ap, int);
			size = 1;
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
#ifdef HAVE_QUAD_T
			if (flags & QUADINT) {
				uqval = va_arg(ap, quad_t);
				if ((quad_t)uqval < 0) {
					uqval = -uqval;
					sign = '-';
				}
			} else
#endif /* HAVE_QUAD_T */
			  {
				ulval = SARG();
				if ((long)ulval < 0) {
					ulval = -ulval;
					sign = '-';
				}
			  }
			base = 10;
			goto number;
#ifdef VSNPRINTF_FLOATING_POINT
		case 'e':		/* anomalous precision */
		case 'E':
			prec = (prec == -1) ?
				DEFPREC + 1 : prec + 1;
			/* FALLTHROUGH */
			goto fp_begin;
		case 'f':		/* always print trailing zeroes */
			if (prec != 0)
				flags |= ALT;
		case 'g':
		case 'G':
			if (prec == -1)
				prec = DEFPREC;
fp_begin:		_double = va_arg(ap, double);
			/* do this before tricky precision changes */
			if (isinf(_double)) {
				if (_double < 0)
					sign = '-';
				cp = "Inf";
				size = 3;
				break;
			}
			if (isnan(_double)) {
				cp = "NaN";
				size = 3;
				break;
			}
			flags |= FPT;
			cp = cvt(_double, prec, flags, &softsign,
				&expt, ch, &ndig);
			if (ch == 'g' || ch == 'G') {
				if (expt <= -4 || expt > prec)
					ch = (ch == 'g') ? 'e' : 'E';
				else
					ch = 'g';
			} 
			if (ch <= 'e') {	/* 'e' or 'E' fmt */
				--expt;
				expsize = exponent(expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
			} else if (ch == 'f') {		/* f fmt */
				if (expt > 0) {
					size = expt;
					if (prec || flags & ALT)
						size += prec + 1;
				} else	/* "0.X" */
					size = prec + 2;
			} else if (expt >= ndig) {	/* fixed g fmt */
				size = expt;
				if (flags & ALT)
					++size;
			} else
				size = ndig + (expt > 0 ?
					1 : 2 - expt);

			if (softsign)
				sign = '-';
			break;
#endif /* VSNPRINTF_FLOATING_POINT */
		case 'n':
#ifdef HAVE_QUAD_T
			if (flags & QUADINT)
				*va_arg(ap, quad_t *) = ret;
			else
#endif /* HAVE_QUAD_T */
			  if (flags & LONGINT)
				*va_arg(ap, long *) = ret;
			else if (flags & SHORTINT)
				*va_arg(ap, short *) = ret;
			else
				*va_arg(ap, int *) = ret;
			continue;	/* no output */
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
#ifdef HAVE_QUAD_T
			if (flags & QUADINT)
				uqval = va_arg(ap, u_quad_t);
			else
#endif /* HAVE_QUAD_T */
				ulval = UARG();
			base = 8;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			ulval = (u_long)va_arg(ap, void *);
			base = 16;
			xdigs = "0123456789abcdef";
			flags = (flags & ~QUADINT) | HEXPREFIX;
			ch = 'x';
			goto nosign;
		case 's':
			if ((cp = va_arg(ap, char *)) == NULL)
				cp = "(null)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p = memchr(cp, 0, prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen(cp);
			sign = '\0';
			break;
#ifdef VSNPRINTF_ERRMSG_CONV
		case 'm':
			/* '%m' is like '%s', except the string is the error
			 * message for current errno rather than an argument.
			 */
			cp = error_message(errno);
			if (cp == NULL)
				cp = "(unknown error)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p = memchr(cp, 0, prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen(cp);
			sign = '\0';
			break;
#endif /* VSNPRINTF_ERRMSG_CONV */
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
#ifdef HAVE_QUAD_T
			if (flags & QUADINT)
				uqval = va_arg(ap, u_quad_t);
			else
#endif /* HAVE_QUAD_T */
				ulval = UARG();
			base = 10;
			goto nosign;
		case 'X':
			xdigs = "0123456789ABCDEF";
			goto hex;
		case 'x':
			xdigs = "0123456789abcdef";
hex:
#ifdef HAVE_QUAD_T
			if (flags & QUADINT)
				uqval = va_arg(ap, u_quad_t);
			else
#endif /* HAVE_QUAD_T */
				ulval = UARG();
			base = 16;
			/* leading 0x/X only if non-zero */
#ifdef HAVE_QUAD_T
			if (flags & ALT &&
			    (flags & QUADINT ? uqval != 0 : ulval != 0))
				flags |= HEXPREFIX;
#else /* not HAVE_QUAD_T */
			if (flags & ALT && (ulval != 0))
				flags |= HEXPREFIX;
#endif /* not HAVE_QUAD_T */

			/* unsigned conversions */
nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
#ifdef HAVE_QUAD_T
			if (flags & QUADINT) {
				if (uqval != 0 || prec != 0)
					cp = __uqtoa(uqval, cp, base,
					    flags & ALT, xdigs);
			} else
#endif /* HAVE_QUAD_T */
			  {
				if (ulval != 0 || prec != 0)
					cp = __ultoa(ulval, cp, base,
					    flags & ALT, xdigs);
			  }
			size = buf + BUF - cp;
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 *
		 * Compute actual size, so we know how much to pad.
		 * fieldsz excludes decimal prec; realsz includes it.
		 */
		fieldsz = size;
		if (sign)
			fieldsz++;
		else if (flags & HEXPREFIX)
			fieldsz += 2;
		realsz = dprec > fieldsz ? dprec : fieldsz;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD(width - realsz, blanks);

		/* prefix */
		if (sign) {
			PRINT(&sign, 1);
		} else if (flags & HEXPREFIX) {
			ox[0] = '0';
			ox[1] = ch;
			PRINT(ox, 2);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD(width - realsz, zeroes);

		/* leading zeroes from decimal precision */
		PAD(dprec - fieldsz, zeroes);

		/* the string or number proper */
#ifdef VSNPRINTF_FLOATING_POINT
		if ((flags & FPT) == 0) {
			PRINT(cp, size);
		} else {	/* glue together f_p fragments */
			if (ch >= 'f') {	/* 'f' or 'g' */
				if (_double == 0) {
				/* kludge for __dtoa irregularity */
					if (prec == 0 ||
					    (flags & ALT) == 0) {
						PRINT("0", 1);
					} else {
						PRINT("0.", 2);
						PAD(ndig - 1, zeroes);
					}
				} else if (expt <= 0) {
					PRINT("0.", 2);
					PAD(-expt, zeroes);
					PRINT(cp, ndig);
				} else if (expt >= ndig) {
					PRINT(cp, ndig);
					PAD(expt - ndig, zeroes);
					if (flags & ALT)
						PRINT(".", 1);
				} else {
					PRINT(cp, expt);
					cp += expt;
					PRINT(".", 1);
					PRINT(cp, ndig-expt);
				}
			} else {	/* 'e' or 'E' */
				if (ndig > 1 || flags & ALT) {
					ox[0] = *cp++;
					ox[1] = '.';
					PRINT(ox, 2);
					if (_double || (flags & ALT) == 0) {
						PRINT(cp, ndig-1);
					} else	/* 0.[0..] */
						/* __dtoa irregularity */
						PAD(ndig - 1, zeroes);
				} else	/* XeYYY */
					PRINT(cp, 1);
				PRINT(expstr, expsize);
			}
		}
#else
		PRINT(cp, size);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD(width - realsz, blanks);

		/* finally, adjust ret */
		ret += width > realsz ? width : realsz;

		FLUSH();	/* copy out the I/O vectors */
	}
done:
	FLUSH();
	*f.pos = 0;
	return ret;
	/* NOTREACHED */
}

#ifdef VSNPRINTF_FLOATING_POINT

static char *__dtoa (double, int, int, int *, int *, char **);

static char *cvt(double value, int ndigits, int flags,
		 int *decpt, int ch, int *length, char *sign)
{
	int mode, dsgn;
	char *digits, *bp, *rve;

	if (ch == 'f')
		mode = 3;
	else {
		mode = 2;
	}
	if (value < 0) {
		value = -value;
		*sign = '-';
	} else
		*sign = '\000';
	digits = __dtoa(value, mode, ndigits, decpt, &dsgn, &rve);
	if (flags & ALT) {	/* Print trailing zeros */
		bp = digits + ndigits;
		if (ch == 'f') {
			if (*digits == '0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	}
	*length = rve - digits;
	return (digits);
}

static int exponent(char *p0, int exp, int fmtch)
{
	char *p, *t;
	char expbuf[MAXEXP];

	p = p0;
	*p++ = fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = '-';
	}
	else
		*p++ = '+';
	t = expbuf + MAXEXP;
	if (exp > 9) {
		do {
			*--t = to_char(exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char(exp);
		for (; t < expbuf + MAXEXP; *p++ = *t++);
	}
	else {
		*p++ = '0';
		*p++ = to_char(exp);
	}
	return (p - p0);
}
#endif /* VSNPRINTF_FLOATING_POINT */

/*** 4.4lite/usr/src/lib/libc/stdlib/strtod.c ***/

#ifdef VSNPRINTF_FLOATING_POINT

static char sccsid_strtod[] = "was: @(#)strtod.c	8.1 (Berkeley) 6/4/93";

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* Please send bug reports to
	David M. Gay
	AT&T Bell Laboratories, Room 2C-463
	600 Mountain Avenue
	Murray Hill, NJ 07974-2070
	U.S.A.
	dmg@research.att.com or research!dmg
 */

/* strtod for IEEE-, VAX-, and IBM-arithmetic machines.
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *	1. We only require IEEE, IBM, or VAX double-precision
 *		arithmetic (not IEEE double-extended).
 *	2. We get by with floating-point arithmetic in a case that
 *		Clinger missed -- when we're computing d * 10^n
 *		for a small integer d and the integer n is not too
 *		much larger than 22 (the maximum integer k for which
 *		we can represent 10^k exactly), we may be able to
 *		compute (d*10^k) * 10^(e-k) with just one roundoff.
 *	3. Rather than a bit-at-a-time adjustment of the binary
 *		result in the hard case, we use floating-point
 *		arithmetic to determine the adjustment to within
 *		one bit; only in really hard cases do we need to
 *		compute a second residual.
 *	4. Because of 3., we don't need a large table of powers of 10
 *		for ten-to-e (just some small tables, e.g. of 10^k
 *		for 0 <= k <= 22).
 */

/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *	significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *	significant byte has the lowest address.
 * #define Sudden_Underflow for IEEE-format machines without gradual
 *	underflow (i.e., that flush to zero on underflow).
 * #define IBM for IBM mainframe-style floating-point arithmetic.
 * #define VAX for VAX-style floating-point arithmetic.
 * #define Unsigned_Shifts if >> does treats its left operand as unsigned.
 * #define No_leftright to omit left-right logic in fast floating-point
 *	computation of dtoa.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3.
 * #define ROUND_BIASED for IEEE-format with biased rounding.
 * #define Inaccurate_Divide for IEEE-format with correctly rounded
 *	products but inaccurate quotients, e.g., for Intel i860.
 * #define Just_16 to store 16 bits per 32-bit long when doing high-precision
 *	integer arithmetic.  Whether this speeds things up or slows things
 *	down depends on the machine and the number being converted.
 * #define Bad_float_h if your system lacks a float.h or if it does not
 *	define some or all of DBL_DIG, DBL_MAX_10_EXP, DBL_MAX_EXP,
 *	FLT_RADIX, FLT_ROUNDS, and DBL_MAX.
 *
 * #define HAVE_RND_PROD and HAVE_RND_QUOT to use rnd_prod and rnd_quot
 *	(assembly routines that use extended-precision instructions to
 *	compute rounded products and quotients) with IBM. 
 */

#if defined(i386) || defined(mips) && defined(MIPSEL)
#define IEEE_8087
#else
#define IEEE_MC68k
#endif

#ifdef VSNPRINTF_DEBUG
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

#ifdef Bad_float_h
#undef __STDC__
#ifdef IEEE_MC68k
#define IEEE_ARITHMETIC
#endif
#ifdef IEEE_8087
#define IEEE_ARITHMETIC
#endif
#ifdef IEEE_ARITHMETIC
#define DBL_DIG 15
#define DBL_MAX_10_EXP 308
#define DBL_MAX_EXP 1024
#define FLT_RADIX 2
#define FLT_ROUNDS 1
#define DBL_MAX 1.7976931348623157e+308
#endif

#ifdef IBM
#define DBL_DIG 16
#define DBL_MAX_10_EXP 75
#define DBL_MAX_EXP 63
#define FLT_RADIX 16
#define FLT_ROUNDS 0
#define DBL_MAX 7.2370055773322621e+75
#endif

#ifdef VAX
#define DBL_DIG 16
#define DBL_MAX_10_EXP 38
#define DBL_MAX_EXP 127
#define FLT_RADIX 2
#define FLT_ROUNDS 1
#define DBL_MAX 1.7014118346046923e+38
#endif

#ifndef LONG_MAX
#define LONG_MAX 2147483647
#endif
#endif /* Bad_float_h */

#ifdef Unsigned_Shifts
#define Sign_Extend(a,b) if (b < 0) a |= 0xffff0000;
#else
#define Sign_Extend(a,b) /*no-op*/
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) + defined(VAX) + defined(IBM) != 1
Exactly one of IEEE_8087, IEEE_MC68k, VAX, or IBM should be defined.
#endif

#ifdef IEEE_8087
#define word0(x) ((unsigned long *)&x)[1]
#define word1(x) ((unsigned long *)&x)[0]
#else
#define word0(x) ((unsigned long *)&x)[0]
#define word1(x) ((unsigned long *)&x)[1]
#endif

/* The following definition of Storeinc is appropriate for MIPS processors.
 * An alternative that might be better on some machines is
 * #define Storeinc(a,b,c) (*a++ = b << 16 | c & 0xffff)
 */
#if defined(IEEE_8087) + defined(VAX)
#define Storeinc(a,b,c) (((unsigned short *)a)[1] = (unsigned short)b, \
((unsigned short *)a)[0] = (unsigned short)c, a++)
#else
#define Storeinc(a,b,c) (((unsigned short *)a)[0] = (unsigned short)b, \
((unsigned short *)a)[1] = (unsigned short)c, a++)
#endif

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#if defined(IEEE_8087) + defined(IEEE_MC68k)
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Bias 1023
#define IEEE_Arith
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#define Infinite(x) (word0(x) == 0x7ff00000) /* sufficient test for here */
#else
#undef  Sudden_Underflow
#define Sudden_Underflow
#ifdef IBM
#define Exp_shift  24
#define Exp_shift1 24
#define Exp_msk1   0x1000000
#define Exp_msk11  0x1000000
#define Exp_mask  0x7f000000
#define P 14
#define Bias 65
#define Exp_1  0x41000000
#define Exp_11 0x41000000
#define Ebits 8	/* exponent has 7 bits, but 8 is the right value in b2d */
#define Frac_mask  0xffffff
#define Frac_mask1 0xffffff
#define Bletch 4
#define Ten_pmax 22
#define Bndry_mask  0xefffff
#define Bndry_mask1 0xffffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 4
#define Tiny0 0x100000
#define Tiny1 0
#define Quick_max 14
#define Int_max 15
#else /* VAX */
#define Exp_shift  23
#define Exp_shift1 7
#define Exp_msk1    0x80
#define Exp_msk11   0x800000
#define Exp_mask  0x7f80
#define P 56
#define Bias 129
#define Exp_1  0x40800000
#define Exp_11 0x4080
#define Ebits 8
#define Frac_mask  0x7fffff
#define Frac_mask1 0xffff007f
#define Ten_pmax 24
#define Bletch 2
#define Bndry_mask  0xffff007f
#define Bndry_mask1 0xffff007f
#define LSB 0x10000
#define Sign_bit 0x8000
#define Log2P 1
#define Tiny0 0x80
#define Tiny1 0
#define Quick_max 15
#define Int_max 15
#endif
#endif

#ifndef IEEE_Arith
#define ROUND_BIASED
#endif

#ifdef HAVE_RND_PROD
#define rounded_product(a,b) a = rnd_prod(a, b)
#else
#define rounded_product(a,b) a *= b
#endif
#ifdef HAVE_RND_QUOT
#define rounded_quotient(a,b) a = rnd_quot(a, b)
#else
#define rounded_quotient(a,b) a /= b
#endif

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

#ifndef Just_16
/* When Pack_32 is not defined, we store 16 bits per 32-bit long.
 * This makes some inner loops simpler and sometimes saves work
 * during multiplications, but it often seems to make things slightly
 * slower.  Hence the default is now to store 32 bits per long.
 */
#ifndef Pack_32
#define Pack_32
#endif
#endif

#define Kmax 15

struct Bigint {
	struct Bigint *next;
	int k, maxwds, sign, wds;
	unsigned long x[1];
};

typedef struct Bigint Bigint;

static Bigint *freelist[Kmax+1];

static Bigint *Balloc(int k)
{
	int x;
	Bigint *rv;

	rv = freelist[k];
	if (rv) {
		freelist[k] = rv->next;
	} else {
		x = 1 << k;
		rv = (Bigint *)malloc(sizeof(Bigint) + (x-1)*sizeof(long));
		rv->k = k;
		rv->maxwds = x;
	}
	rv->sign = rv->wds = 0;
	return rv;
}

static void Bfree(Bigint *v)
{
	if (v) {
		v->next = freelist[v->k];
		freelist[v->k] = v;
	}
}

#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign, \
y->wds*sizeof(long) + 2*sizeof(int))

/* multiply by m and add a */
static Bigint *multadd(Bigint *b, int m, int a)
{
	int i, wds;
	unsigned long *x, y;
#ifdef Pack_32
	unsigned long xi, z;
#endif
	Bigint *b1;

	wds = b->wds;
	x = b->x;
	i = 0;
	do {
#ifdef Pack_32
		xi = *x;
		y = (xi & 0xffff) * m + a;
		z = (xi >> 16) * m + (y >> 16);
		a = (int)(z >> 16);
		*x++ = (z << 16) + (y & 0xffff);
#else
		y = *x * m + a;
		a = (int)(y >> 16);
		*x++ = y & 0xffff;
#endif
	} while (++i < wds);
	if (a) {
		if (wds >= b->maxwds) {
			b1 = Balloc(b->k+1);
			Bcopy(b1, b);
			Bfree(b);
			b = b1;
			}
		b->x[wds++] = a;
		b->wds = wds;
	}
	return b;
}

static int hi0bits(unsigned long x)
{
	int k = 0;

	if (!(x & 0xffff0000)) {
		k = 16;
		x <<= 16;
	}
	if (!(x & 0xff000000)) {
		k += 8;
		x <<= 8;
	}
	if (!(x & 0xf0000000)) {
		k += 4;
		x <<= 4;
	}
	if (!(x & 0xc0000000)) {
		k += 2;
		x <<= 2;
	}
	if (!(x & 0x80000000)) {
		k++;
		if (!(x & 0x40000000))
			return 32;
	}
	return k;
}

static int lo0bits(unsigned long *y)
{
	int k;
	unsigned long x = *y;

	if (x & 7) {
		if (x & 1)
			return 0;
		if (x & 2) {
			*y = x >> 1;
			return 1;
		}
		*y = x >> 2;
		return 2;
	}
	k = 0;
	if (!(x & 0xffff)) {
		k = 16;
		x >>= 16;
	}
	if (!(x & 0xff)) {
		k += 8;
		x >>= 8;
	}
	if (!(x & 0xf)) {
		k += 4;
		x >>= 4;
	}
	if (!(x & 0x3)) {
		k += 2;
		x >>= 2;
	}
	if (!(x & 1)) {
		k++;
		x >>= 1;
		if (!x & 1)
			return 32;
	}
	*y = x;
	return k;
}

static Bigint *i2b(int i)
{
	Bigint *b;

	b = Balloc(1);
	b->x[0] = i;
	b->wds = 1;
	return b;
}

static Bigint *mult(Bigint *a, Bigint *b)
{
	Bigint *c;
	int k, wa, wb, wc;
	unsigned long carry, y, z;
	unsigned long *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
#ifdef Pack_32
	unsigned long z2;
#endif

	if (a->wds < b->wds) {
		c = a;
		a = b;
		b = c;
	}
	k = a->k;
	wa = a->wds;
	wb = b->wds;
	wc = wa + wb;
	if (wc > a->maxwds)
		k++;
	c = Balloc(k);
	for (x = c->x, xa = x + wc; x < xa; x++)
		*x = 0;
	xa = a->x;
	xae = xa + wa;
	xb = b->x;
	xbe = xb + wb;
	xc0 = c->x;
#ifdef Pack_32
	for (; xb < xbe; xb++, xc0++) {
		y = *xb & 0xffff;
		if (y) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = (*x & 0xffff) * y + (*xc & 0xffff) + carry;
				carry = z >> 16;
				z2 = (*x++ >> 16) * y + (*xc >> 16) + carry;
				carry = z2 >> 16;
				Storeinc(xc, z2, z);
			} while (x < xae);
			*xc = carry;
		}
		y = *xb >> 16;
		if (y) {
			x = xa;
			xc = xc0;
			carry = 0;
			z2 = *xc;
			do {
				z = (*x & 0xffff) * y + (*xc >> 16) + carry;
				carry = z >> 16;
				Storeinc(xc, z, z2);
				z2 = (*x++ >> 16) * y + (*xc & 0xffff) + carry;
				carry = z2 >> 16;
			} while (x < xae);
			*xc = z2;
		}
	}
#else
	for (; xb < xbe; xc0++) {
		if (y = *xb++) {
			x = xa;
			xc = xc0;
			carry = 0;
			do {
				z = *x++ * y + *xc + carry;
				carry = z >> 16;
				*xc++ = z & 0xffff;
			} while (x < xae);
			*xc = carry;
		}
	}
#endif
	for (xc0 = c->x, xc = xc0 + wc; wc > 0 && !*--xc; --wc) ;
	c->wds = wc;
	return c;
}

static Bigint *p5s;

static Bigint *pow5mult(Bigint *b, int k)
{
	Bigint *b1, *p5, *p51;
	int i;
	static int p05[3] = { 5, 25, 125 };

	i = k & 3;
	if (i)
		b = multadd(b, p05[i-1], 0);

	if (!(k >>= 2))
		return b;
	if (!(p5 = p5s)) {
		/* first time */
		p5 = p5s = i2b(625);
		p5->next = 0;
	}
	for (;;) {
		if (k & 1) {
			b1 = mult(b, p5);
			Bfree(b);
			b = b1;
		}
		if (!(k >>= 1))
			break;
		if (!(p51 = p5->next)) {
			p51 = p5->next = mult(p5,p5);
			p51->next = 0;
		}
		p5 = p51;
	}
	return b;
}

static Bigint *lshift(Bigint *b, int k)
{
	int i, k1, n, n1;
	Bigint *b1;
	unsigned long *x, *x1, *xe, z;

#ifdef Pack_32
	n = k >> 5;
#else
	n = k >> 4;
#endif
	k1 = b->k;
	n1 = n + b->wds + 1;
	for (i = b->maxwds; n1 > i; i <<= 1)
		k1++;
	b1 = Balloc(k1);
	x1 = b1->x;
	for (i = 0; i < n; i++)
		*x1++ = 0;
	x = b->x;
	xe = x + b->wds;
#ifdef Pack_32
	if (k &= 0x1f) {
		k1 = 32 - k;
		z = 0;
		do {
			*x1++ = *x << k | z;
			z = *x++ >> k1;
		} while (x < xe);
		*x1 = z;
		if (z)
			++n1;
	}
#else
	if (k &= 0xf) {
		k1 = 16 - k;
		z = 0;
		do {
			*x1++ = *x << k  & 0xffff | z;
			z = *x++ >> k1;
		} while (x < xe);
		if (*x1 = z)
			++n1;
	}
#endif
	else
		do
			*x1++ = *x++;
		while (x < xe);
	b1->wds = n1 - 1;
	Bfree(b);
	return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
	unsigned long *xa, *xa0, *xb, *xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
#ifdef VSNPRINTF_DEBUG
	if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
	if (i -= j)
		return i;
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for (;;) {
		if (*--xa != *--xb)
			return *xa < *xb ? -1 : 1;
		if (xa <= xa0)
			break;
	}
	return 0;
}

static Bigint *diff(Bigint *a, Bigint *b)
{
	Bigint *c;
	int i, wa, wb;
	long borrow, y;	/* We need signed shifts here. */
	unsigned long *xa, *xae, *xb, *xbe, *xc;
#ifdef Pack_32
	long z;
#endif

	i = cmp(a,b);
	if (!i) {
		c = Balloc(0);
		c->wds = 1;
		c->x[0] = 0;
		return c;
	}
	if (i < 0) {
		c = a;
		a = b;
		b = c;
		i = 1;
	} else
		i = 0;
	c = Balloc(a->k);
	c->sign = i;
	wa = a->wds;
	xa = a->x;
	xae = xa + wa;
	wb = b->wds;
	xb = b->x;
	xbe = xb + wb;
	xc = c->x;
	borrow = 0;
#ifdef Pack_32
	do {
		y = (*xa & 0xffff) - (*xb & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) - (*xb++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
	} while (xb < xbe);
	while (xa < xae) {
		y = (*xa & 0xffff) + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		z = (*xa++ >> 16) + borrow;
		borrow = z >> 16;
		Sign_Extend(borrow, z);
		Storeinc(xc, z, y);
	}
#else
	do {
		y = *xa++ - *xb++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
	} while (xb < xbe);
	while (xa < xae) {
		y = *xa++ + borrow;
		borrow = y >> 16;
		Sign_Extend(borrow, y);
		*xc++ = y & 0xffff;
	}
#endif
	while (!*--xc)
		wa--;
	c->wds = wa;
	return c;
}

static Bigint *d2b(double d, int *e, int *bits)
{
	Bigint *b;
	int de, i, k;
	unsigned long *x, y, z;
#ifdef VAX
	unsigned long d0, d1;
	d0 = word0(d) >> 16 | word0(d) << 16;
	d1 = word1(d) >> 16 | word1(d) << 16;
#else
#define d0 word0(d)
#define d1 word1(d)
#endif

#ifdef Pack_32
	b = Balloc(1);
#else
	b = Balloc(2);
#endif
	x = b->x;

	z = d0 & Frac_mask;
	d0 &= 0x7fffffff;	/* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
	de = (int)(d0 >> Exp_shift);
#ifndef IBM
	z |= Exp_msk11;
#endif
#else
	de = (int)(d0 >> Exp_shift);
	if (de)
		z |= Exp_msk1;
#endif
#ifdef Pack_32
	y = d1;
	if (y) {
		k = lo0bits(&y);
		if (k) {
			x[0] = y | (z << (32 - k));
			z >>= k;
			}
		else
			x[0] = y;
		i = b->wds = (x[1] = z) ? 2 : 1;
	} else {
#ifdef VSNPRINTF_DEBUG
		if (!z)
			Bug("Zero passed to d2b");
#endif
		k = lo0bits(&z);
		x[0] = z;
		i = b->wds = 1;
		k += 32;
	}
#else
	if (y = d1) {
		if (k = lo0bits(&y))
			if (k >= 16) {
				x[0] = y | z << 32 - k & 0xffff;
				x[1] = z >> k - 16 & 0xffff;
				x[2] = z >> k;
				i = 2;
			} else {
				x[0] = y & 0xffff;
				x[1] = y >> 16 | z << 16 - k & 0xffff;
				x[2] = z >> k & 0xffff;
				x[3] = z >> k+16;
				i = 3;
			}
		else {
			x[0] = y & 0xffff;
			x[1] = y >> 16;
			x[2] = z & 0xffff;
			x[3] = z >> 16;
			i = 3;
		}
	} else {
#ifdef VSNPRINTF_DEBUG
		if (!z)
			Bug("Zero passed to d2b");
#endif
		k = lo0bits(&z);
		if (k >= 16) {
			x[0] = z;
			i = 0;
		} else {
			x[0] = z & 0xffff;
			x[1] = z >> 16;
			i = 1;
		}
		k += 32;
	}
	while (!x[i])
		--i;
	b->wds = i + 1;
#endif
#ifndef Sudden_Underflow
	if (de) {
#endif
#ifdef IBM
		*e = (de - Bias - (P-1) << 2) + k;
		*bits = 4*P + 8 - k - hi0bits(word0(d) & Frac_mask);
#else
		*e = de - Bias - (P-1) + k;
		*bits = P - k;
#endif
#ifndef Sudden_Underflow
	} else {
		*e = de - Bias - (P-1) + 1 + k;
#ifdef Pack_32
		*bits = 32*i - hi0bits(x[i-1]);
#else
		*bits = (i+2)*16 - hi0bits(x[i]);
#endif
	}
#endif
	return b;
}
#undef d0
#undef d1

static double tens[] = {
		1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
		1e20, 1e21, 1e22
#ifdef VAX
		, 1e23, 1e24
#endif
		};

#ifdef IEEE_Arith
static double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
#define n_bigtens 5
#else
#ifdef IBM
static double bigtens[] = { 1e16, 1e32, 1e64 };
#define n_bigtens 3
#else
static double bigtens[] = { 1e16, 1e32 };
#define n_bigtens 2
#endif
#endif

static int quorem(Bigint *b, Bigint *S)
{
	int n;
	long borrow, y;
	unsigned long carry, q, ys;
	unsigned long *bx, *bxe, *sx, *sxe;
#ifdef Pack_32
	long z;
	unsigned long si, zs;
#endif

	n = S->wds;
#ifdef VSNPRINTF_DEBUG
	/*debug*/ if (b->wds > n)
	/*debug*/	Bug("oversize b in quorem");
#endif
	if (b->wds < n)
		return 0;
	sx = S->x;
	sxe = sx + --n;
	bx = b->x;
	bxe = bx + n;
	q = *bxe / (*sxe + 1);	/* ensure q <= true quotient */
#ifdef VSNPRINTF_DEBUG
	/*debug*/ if (q > 9)
	/*debug*/	Bug("oversized quotient in quorem");
#endif
	if (q) {
		borrow = 0;
		carry = 0;
		do {
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) * q + carry;
			zs = (si >> 16) * q + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
#else
			ys = *sx++ * q + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
		} while (sx <= sxe);
		if (!*bxe) {
			bx = b->x;
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	if (cmp(b, S) >= 0) {
		q++;
		borrow = 0;
		carry = 0;
		bx = b->x;
		sx = S->x;
		do {
#ifdef Pack_32
			si = *sx++;
			ys = (si & 0xffff) + carry;
			zs = (si >> 16) + (ys >> 16);
			carry = zs >> 16;
			y = (*bx & 0xffff) - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			z = (*bx >> 16) - (zs & 0xffff) + borrow;
			borrow = z >> 16;
			Sign_Extend(borrow, z);
			Storeinc(bx, z, y);
#else
			ys = *sx++ + carry;
			carry = ys >> 16;
			y = *bx - (ys & 0xffff) + borrow;
			borrow = y >> 16;
			Sign_Extend(borrow, y);
			*bx++ = y & 0xffff;
#endif
		} while (sx <= sxe);
		bx = b->x;
		bxe = bx + n;
		if (!*bxe) {
			while (--bxe > bx && !*bxe)
				--n;
			b->wds = n;
		}
	}
	return q;
}

/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *	1. Rather than iterating, we use a simple numeric overestimate
 *	   to determine k = floor(log10(d)).  We scale relevant
 *	   quantities using O(log2(k)) rather than O(k) multiplications.
 *	2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *	   try to generate digits strictly left to right.  Instead, we
 *	   compute with fewer bits and propagate the carry if necessary
 *	   when rounding the final digit up.  This is often faster.
 *	3. Under the assumption that input will be rounded nearest,
 *	   mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *	   That is, we allow equality in stopping tests when the
 *	   round-nearest rule will give the same floating-point value
 *	   as would satisfaction of the stopping test with strict
 *	   inequality.
 *	4. We remove common factors of powers of 2 from relevant
 *	   quantities.
 *	5. When converting floating-point integers less than 1e16,
 *	   we use floating-point arithmetic rather than resorting
 *	   to multiple-precision integers.
 *	6. When asked to produce fewer than 15 digits, we first try
 *	   to get by with floating-point arithmetic; we resort to
 *	   multiple-precision integer arithmetic only if we cannot
 *	   guarantee that the floating-point calculation has given
 *	   the correctly rounded result.  For k requested digits and
 *	   "uniformly" distributed input, the probability is
 *	   something like 10^(k-15) that we must resort to the long
 *	   calculation.
 */

char *__dtoa(double d, int mode, int ndigits,
	     int *decpt, int *sign, char **rve)
{
 /*	Arguments ndigits, decpt, sign are similar to those
	of ecvt and fcvt; trailing zeros are suppressed from
	the returned string.  If not null, *rve is set to point
	to the end of the return value.  If d is +-Infinity or NaN,
	then *decpt is set to 9999.

	mode:
		0 ==> shortest string that yields d when read in
			and rounded to nearest.
		1 ==> like 0, but with Steele & White stopping rule;
			e.g. with IEEE P754 arithmetic , mode 0 gives
			1e23 whereas mode 1 gives 9.999999999999999e22.
		2 ==> max(1,ndigits) significant digits.  This gives a
			return value similar to that of ecvt, except
			that trailing zeros are suppressed.
		3 ==> through ndigits past the decimal point.  This
			gives a return value similar to that from fcvt,
			except that trailing zeros are suppressed, and
			ndigits can be negative.
		4-9 should give the same return values as 2-3, i.e.,
			4 <= mode <= 9 ==> same return as mode
			2 + (mode & 1).  These modes are mainly for
			debugging; often they run slower but sometimes
			faster than modes 2-3.
		4,5,8,9 ==> left-to-right digit generation.
		6-9 ==> don't try fast floating-point estimate
			(if applicable).

		Values of mode other than 0-9 are treated as mode 0.

		Sufficient space is allocated to the return value
		to hold the suppressed trailing zeros.
	*/

	int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1,
		j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
		spec_case, try_quick;
	long L;
#ifndef Sudden_Underflow
	int denorm;
	unsigned long x;
#endif
	Bigint *b, *b1, *delta, *mlo, *mhi, *S;
	double d2, ds, eps;
	char *s, *s0;
	static Bigint *result;
	static int result_k;

	if (result) {
		result->k = result_k;
		result->maxwds = 1 << result_k;
		Bfree(result);
		result = 0;
	}

	if (word0(d) & Sign_bit) {
		/* set sign for everything, including 0's and NaNs */
		*sign = 1;
		word0(d) &= ~Sign_bit;	/* clear sign bit */
	}
	else
		*sign = 0;

#if defined(IEEE_Arith) + defined(VAX)
#ifdef IEEE_Arith
	if ((word0(d) & Exp_mask) == Exp_mask)
#else
	if (word0(d)  == 0x8000)
#endif
	{
		/* Infinity or NaN */
		*decpt = 9999;
		s =
#ifdef IEEE_Arith
			!word1(d) && !(word0(d) & 0xfffff) ? "Infinity" :
#endif
				"NaN";
		if (rve)
			*rve =
#ifdef IEEE_Arith
				s[3] ? s + 8 :
#endif
						s + 3;
		return s;
	}
#endif
#ifdef IBM
	d += 0; /* normalize */
#endif
	if (!d) {
		*decpt = 1;
		s = "0";
		if (rve)
			*rve = s + 1;
		return s;
	}

	b = d2b(d, &be, &bbits);
	i = (int)(word0(d) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#ifndef Sudden_Underflow
	if (i) {
#endif
		d2 = d;
		word0(d2) &= Frac_mask1;
		word0(d2) |= Exp_11;
#ifdef IBM
		if (j = 11 - hi0bits(word0(d2) & Frac_mask))
			d2 /= 1 << j;
#endif

		/* log(x)	~=~ log(1.5) + (x-1.5)/1.5
		 * log10(x)	 =  log(x) / log(10)
		 *		~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
		 * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
		 *
		 * This suggests computing an approximation k to log10(d) by
		 *
		 * k = (i - Bias)*0.301029995663981
		 *	+ ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
		 *
		 * We want k to be too large rather than too small.
		 * The error in the first-order Taylor series approximation
		 * is in our favor, so we just round up the constant enough
		 * to compensate for any error in the multiplication of
		 * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
		 * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
		 * adding 1e-13 to the constant term more than suffices.
		 * Hence we adjust the constant term to 0.1760912590558.
		 * (We could get a more accurate k by invoking log10,
		 *  but this is probably not worthwhile.)
		 */

		i -= Bias;
#ifdef IBM
		i <<= 2;
		i += j;
#endif
#ifndef Sudden_Underflow
		denorm = 0;
	} else {
		/* d is denormalized */

		i = bbits + be + (Bias + (P-1) - 1);
		x = (i > 32)  ? (word0(d) << (64 - i)) | (word1(d) >> (i - 32))
			      : (word1(d) << (32 - i));
		d2 = x;
		word0(d2) -= 31*Exp_msk1; /* adjust exponent */
		i -= (Bias + (P-1) - 1) + 1;
		denorm = 1;
	}
#endif
	ds = (d2-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
	k = (int)ds;
	if (ds < 0. && ds != k)
		k--;	/* want k = floor(ds) */
	k_check = 1;
	if (k >= 0 && k <= Ten_pmax) {
		if (d < tens[k])
			k--;
		k_check = 0;
	}
	j = bbits - i - 1;
	if (j >= 0) {
		b2 = 0;
		s2 = j;
	} else {
		b2 = -j;
		s2 = 0;
	}
	if (k >= 0) {
		b5 = 0;
		s5 = k;
		s2 += k;
	} else {
		b2 -= k;
		b5 = -k;
		s5 = 0;
	}
	if (mode < 0 || mode > 9)
		mode = 0;
	try_quick = 1;
	if (mode > 5) {
		mode -= 4;
		try_quick = 0;
	}
	leftright = 1;
	switch(mode) {
		case 0:
		case 1:
			ilim = ilim1 = -1;
			i = 18;
			ndigits = 0;
			break;
		case 2:
			leftright = 0;
			/* no break */
		case 4:
			if (ndigits <= 0)
				ndigits = 1;
			ilim = ilim1 = i = ndigits;
			break;
		case 3:
			leftright = 0;
			/* no break */
		case 5:
			i = ndigits + k + 1;
			ilim = i;
			ilim1 = i - 1;
			if (i <= 0)
				i = 1;
	}
	j = sizeof(unsigned long);
	for (result_k = 0; sizeof(Bigint) - sizeof(unsigned long) + j < i;
		j <<= 1) result_k++;
	result = Balloc(result_k);
	s = s0 = (char *)result;

	if (ilim >= 0 && ilim <= Quick_max && try_quick) {

		/* Try to get by with floating-point arithmetic. */

		i = 0;
		d2 = d;
		k0 = k;
		ilim0 = ilim;
		ieps = 2; /* conservative */
		if (k > 0) {
			ds = tens[k&0xf];
			j = k >> 4;
			if (j & Bletch) {
				/* prevent overflows */
				j &= Bletch - 1;
				d /= bigtens[n_bigtens-1];
				ieps++;
			}
			for (; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					ds *= bigtens[i];
				}
			d /= ds;
		} else if (j1 = -k) {
			d *= tens[j1 & 0xf];
			for (j = j1 >> 4; j; j >>= 1, i++)
				if (j & 1) {
					ieps++;
					d *= bigtens[i];
				}
		}
		if (k_check && d < 1. && ilim > 0) {
			if (ilim1 <= 0)
				goto fast_failed;
			ilim = ilim1;
			k--;
			d *= 10.;
			ieps++;
		}
		eps = ieps*d + 7.;
		word0(eps) -= (P-1)*Exp_msk1;
		if (ilim == 0) {
			S = mhi = 0;
			d -= 5.;
			if (d > eps)
				goto one_digit;
			if (d < -eps)
				goto no_digits;
			goto fast_failed;
		}
#ifndef No_leftright
		if (leftright) {
			/* Use Steele & White method of only
			 * generating digits needed.
			 */
			eps = 0.5/tens[ilim-1] - eps;
			for (i = 0;;) {
				L = d;
				d -= L;
				*s++ = '0' + (int)L;
				if (d < eps)
					goto ret1;
				if (1. - d < eps)
					goto bump_up;
				if (++i >= ilim)
					break;
				eps *= 10.;
				d *= 10.;
			}
		} else {
#endif
			/* Generate ilim digits, then fix them up. */
			eps *= tens[ilim-1];
			for (i = 1;; i++, d *= 10.) {
				L = d;
				d -= L;
				*s++ = '0' + (int)L;
				if (i == ilim) {
					if (d > 0.5 + eps)
						goto bump_up;
					else if (d < 0.5 - eps) {
						while (*--s == '0');
						s++;
						goto ret1;
					}
					break;
				}
			}
#ifndef No_leftright
		}
#endif
 fast_failed:
		s = s0;
		d = d2;
		k = k0;
		ilim = ilim0;
	}

	/* Do we have a "small" integer? */

	if (be >= 0 && k <= Int_max) {
		/* Yes. */
		ds = tens[k];
		if (ndigits < 0 && ilim <= 0) {
			S = mhi = 0;
			if (ilim < 0 || d <= 5*ds)
				goto no_digits;
			goto one_digit;
		}
		for (i = 1;; i++) {
			L = d / ds;
			d -= L*ds;
#ifdef Check_FLT_ROUNDS
			/* If FLT_ROUNDS == 2, L will usually be high by 1 */
			if (d < 0) {
				L--;
				d += ds;
			}
#endif
			*s++ = '0' + (int)L;
			if (i == ilim) {
				d += d;
				if (d > ds || d == ds && L & 1) {
 bump_up:
					while (*--s == '9')
						if (s == s0) {
							k++;
							*s = '0';
							break;
						}
					++*s++;
				}
				break;
			}
			if (!(d *= 10.))
				break;
		}
		goto ret1;
	}

	m2 = b2;
	m5 = b5;
	mhi = mlo = 0;
	if (leftright) {
		if (mode < 2) {
			i =
#ifndef Sudden_Underflow
				denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
#ifdef IBM
				1 + 4*P - 3 - bbits + ((bbits + be - 1) & 3);
#else
				1 + P - bbits;
#endif
		} else {
			j = ilim - 1;
			if (m5 >= j)
				m5 -= j;
			else {
				s5 += j -= m5;
				b5 += j;
				m5 = 0;
			}
			if ((i = ilim) < 0) {
				m2 -= i;
				i = 0;
			}
		}
		b2 += i;
		s2 += i;
		mhi = i2b(1);
	}
	if (m2 > 0 && s2 > 0) {
		i = m2 < s2 ? m2 : s2;
		b2 -= i;
		m2 -= i;
		s2 -= i;
	}
	if (b5 > 0) {
		if (leftright) {
			if (m5 > 0) {
				mhi = pow5mult(mhi, m5);
				b1 = mult(mhi, b);
				Bfree(b);
				b = b1;
				}
			if (j = b5 - m5)
				b = pow5mult(b, j);
		} else
			b = pow5mult(b, b5);
	}
	S = i2b(1);
	if (s5 > 0)
		S = pow5mult(S, s5);

	/* Check for special case that d is a normalized power of 2. */

	if (mode < 2) {
		if (!word1(d) && !(word0(d) & Bndry_mask)
#ifndef Sudden_Underflow
		 && word0(d) & Exp_mask
#endif
				) {
			/* The special case */
			b2 += Log2P;
			s2 += Log2P;
			spec_case = 1;
		} else
			spec_case = 0;
	}

	/* Arrange for convenient computation of quotients:
	 * shift left if necessary so divisor has 4 leading 0 bits.
	 *
	 * Perhaps we should just compute leading 28 bits of S once
	 * and for all and pass them and a shift to quorem, so it
	 * can do shifts and ors to compute the numerator for q.
	 */
#ifdef Pack_32
	if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0x1f)
		i = 32 - i;
#else
	if (i = ((s5 ? 32 - hi0bits(S->x[S->wds-1]) : 1) + s2) & 0xf)
		i = 16 - i;
#endif
	if (i > 4) {
		i -= 4;
		b2 += i;
		m2 += i;
		s2 += i;
	} else if (i < 4) {
		i += 28;
		b2 += i;
		m2 += i;
		s2 += i;
	}
	if (b2 > 0)
		b = lshift(b, b2);
	if (s2 > 0)
		S = lshift(S, s2);
	if (k_check) {
		if (cmp(b,S) < 0) {
			k--;
			b = multadd(b, 10, 0);	/* we botched the k estimate */
			if (leftright)
				mhi = multadd(mhi, 10, 0);
			ilim = ilim1;
		}
	}
	if (ilim <= 0 && mode > 2) {
		if (ilim < 0 || cmp(b,S = multadd(S,5,0)) <= 0) {
			/* no digits, fcvt style */
 no_digits:
			k = -1 - ndigits;
			goto ret;
		}
 one_digit:
		*s++ = '1';
		k++;
		goto ret;
	}
	if (leftright) {
		if (m2 > 0)
			mhi = lshift(mhi, m2);

		/* Compute mlo -- check for special case
		 * that d is a normalized power of 2.
		 */

		mlo = mhi;
		if (spec_case) {
			mhi = Balloc(mhi->k);
			Bcopy(mhi, mlo);
			mhi = lshift(mhi, Log2P);
		}

		for (i = 1;;i++) {
			dig = quorem(b,S) + '0';
			/* Do we yet have the shortest decimal string
			 * that will round to d?
			 */
			j = cmp(b, mlo);
			delta = diff(S, mhi);
			j1 = delta->sign ? 1 : cmp(b, delta);
			Bfree(delta);
#ifndef ROUND_BIASED
			if (j1 == 0 && !mode && !(word1(d) & 1)) {
				if (dig == '9')
					goto round_9_up;
				if (j > 0)
					dig++;
				*s++ = dig;
				goto ret;
			}
#endif
			if (j < 0 || j == 0 && !mode
#ifndef ROUND_BIASED
							&& !(word1(d) & 1)
#endif
					) {
				if (j1 > 0) {
					b = lshift(b, 1);
					j1 = cmp(b, S);
					if ((j1 > 0 || j1 == 0 && dig & 1)
					&& dig++ == '9')
						goto round_9_up;
				}
				*s++ = dig;
				goto ret;
			}
			if (j1 > 0) {
				if (dig == '9') { /* possible if i == 1 */
 round_9_up:
					*s++ = '9';
					goto roundoff;
				}
				*s++ = dig + 1;
				goto ret;
			}
			*s++ = dig;
			if (i == ilim)
				break;
			b = multadd(b, 10, 0);
			if (mlo == mhi)
				mlo = mhi = multadd(mhi, 10, 0);
			else {
				mlo = multadd(mlo, 10, 0);
				mhi = multadd(mhi, 10, 0);
			}
		}
	} else
		for (i = 1;; i++) {
			*s++ = dig = quorem(b,S) + '0';
			if (i >= ilim)
				break;
			b = multadd(b, 10, 0);
		}

	/* Round off last digit */

	b = lshift(b, 1);
	j = cmp(b, S);
	if (j > 0 || j == 0 && dig & 1) {
 roundoff:
		while (*--s == '9')
			if (s == s0) {
				k++;
				*s++ = '1';
				goto ret;
			}
		++*s++;
	} else {
		while (*--s == '0');
		s++;
	}
 ret:
	Bfree(S);
	if (mhi) {
		if (mlo && mlo != mhi)
			Bfree(mlo);
		Bfree(mhi);
	}
 ret1:
	Bfree(b);
	if (s == s0) {	/* don't return empty string */
		*s++ = '0';
		k = 0;
	}
	*s = 0;
	*decpt = k + 1;
	if (rve)
		*rve = s;
	return s0;
	}

#endif /* VSNPRINTF_FLOATING_POINT */
