/*
 * Copyright (c) 1994 Paul Vojta.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * NOTE:
 *	xdvi is based on prior work, as noted in the modification history
 *	in xdvi.c.
 */

#include "xdvi.h"
#include <errno.h>

#ifdef VMS
#include <rmsdef.h>
#endif /* VMS */

#ifdef	X_NOT_STDC_ENV
extern	int	errno;
char	*malloc();
#endif

#if	defined(macII) && !__STDC__ /* stdlib.h doesn't define these */
char	*malloc();
#endif /* macII */

#if	NeedVarargsPrototypes		/* this is for oops */
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef	DOPRNT	/* define this if vfprintf gives you trouble */
#define	vfprintf(stream, message, args)	_doprnt(message, args, stream)
#endif

/*
 *	General utility routines.
 */

/*
 *	Print error message and quit.
 */

#if	NeedVarargsPrototypes
NORETURN void
oops(_Xconst char *message, ...)
#else
/* VARARGS */
NORETURN void
oops(va_alist)
	va_dcl
#endif
{
#if	!NeedVarargsPrototypes
	_Xconst char *message;
#endif
	va_list	args;

	Fprintf(stderr, "%s: ", prog);
#if	NeedVarargsPrototypes
	va_start(args, message);
#else
	va_start(args);
	message = va_arg(args, _Xconst char *);
#endif
	(void) vfprintf(stderr, message, args);
	va_end(args);
	Putc('\n', stderr);
#if	PS
	psp.destroy();
#endif
	exit(1);
}

/*
 *	Either allocate storage or fail with explanation.
 */

char *
xmalloc(size, why)
	unsigned	size;
	_Xconst char	*why;
{
	char *mem = malloc(size);

	if (mem == NULL)
	    oops("! Cannot allocate %u bytes for %s.\n", size, why);
	return mem;
}

/*
 *	Allocate bitmap for given font and character
 */

void
alloc_bitmap(bitmap)
	register struct bitmap *bitmap;
{
	register unsigned int	size;

	/* width must be multiple of 16 bits for raster_op */
	bitmap->bytes_wide = ROUNDUP((int) bitmap->w, BITS_PER_BMUNIT) *
	    BYTES_PER_BMUNIT;
	size = bitmap->bytes_wide * bitmap->h;
	bitmap->bits = xmalloc(size != 0 ? size : 1, "character bitmap");
}


/*
 *	Hopefully a self-explanatory name.  This code assumes the second
 *	argument is lower case.
 */

int
memicmp(s1, s2, n)
	_Xconst char	*s1;
	_Xconst char	*s2;
	size_t		n;
{
	while (n > 0) {
	    int i = tolower(*s1) - *s2;
	    if (i != 0) return i;
	    ++s1;
	    ++s2;
	    --n;
	}
	return 0;
}


/*
 *	Close the pixel file for the least recently used font.
 */

static	void
close_a_file()
{
	register struct font *fontp;
	unsigned short oldest = ~0;
	struct font *f = NULL;

	for (fontp = font_head; fontp != NULL; fontp = fontp->next)
	    if (fontp->file != NULL && fontp->timestamp <= oldest) {
		f = fontp;
		oldest = fontp->timestamp;
	    }
	if (f == NULL)
	    oops("Can't find an open pixel file to close");
	Fclose(f->file);
	f->file = NULL;
	++n_files_left;
}

/*
 *	Open a file in the given mode.
 */

FILE *
#ifndef	VMS
xfopen(filename, type)
	_Xconst char	*filename;
	_Xconst char	*type;
#define	TYPE	type
#else
xfopen(filename, type, type2)
	_Xconst char	*filename;
	_Xconst char	*type;
	_Xconst char	*type2;
#define	TYPE	type, type2
#endif	/* VMS */
{
	FILE	*f;

	if (n_files_left == 0) close_a_file();
	f = fopen(filename, TYPE);
#ifndef	VMS
	if (f == NULL && (errno == EMFILE || errno == ENFILE))
#else	/* VMS */
	if (f == NULL && errno == EVMSERR && vaxc$errno == RMS$_ACC)
#endif	/* VMS */
	{
	    n_files_left = 0;
	    close_a_file();
	    f = fopen(filename, TYPE);
	}
#ifdef	F_SETFD
	if (f != NULL) (void) fcntl(fileno(f), F_SETFD, 1);
#endif
	return f;
}
#undef	TYPE


#ifdef	PS_GS
/*
 *	Create a pipe, closing a file if necessary.  This is (so far) used only
 *	in psgs.c.
 */

int
xpipe(fd)
	int	*fd;
{
	int	retval;

	for (;;) {
	    retval = pipe(fd);
	    if (retval == 0 || (errno != EMFILE && errno != ENFILE)) break;
	    n_files_left = 0;
	    close_a_file();
	}
	return retval;
}
#endif	/* PS_GS */


/*
 *
 *      Read size bytes from the FILE fp, constructing them into a
 *      signed/unsigned integer.
 *
 */

unsigned long
num(fp, size)
	register FILE *fp;
	register int size;
{
	register long x = 0;

	while (size--) x = (x << 8) | one(fp);
	return x;
}

long
snum(fp, size)
	register FILE *fp;
	register int size;
{
	register long x;

#if	__STDC__
	x = (signed char) getc(fp);
#else
	x = (unsigned char) getc(fp);
	if (x & 0x80) x -= 0x100;
#endif
	while (--size) x = (x << 8) | one(fp);
	return x;
}
