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
#include <ctype.h>	/* needed for memicmp() */
#include <pwd.h>

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
 *	Either (re)allocate storage or fail with explanation.
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


char *
xrealloc(where, size, why)
	char		*where;
	unsigned	size;
	_Xconst char	*why;
{
	char	*mem	= realloc(where, size);

	if (mem == NULL)
	    oops("! Cannot relllocate %u bytes for %s.\n", size, why);
	return mem;
}


/*
 *	Allocate a new string.  The second argument is the length, or -1.
 */

char	*
newstring(str, len)
	_Xconst char	*str;
	int		len;
{
	char	*new;

	if (len <= 0) len = strlen(str) + 1;
	new = xmalloc(len, "character string");
	bcopy(str, new, len);
	return new;
}


/*
 *	Expand the matrix *ffline to at least the given size.
 */

void
expandline(n)
	int	n;
{
	int	newlen	= n + 128;

	ffline = (ffline == NULL) ? xmalloc(newlen, "space for file paths")
		: xrealloc(ffline, newlen, "space for file paths");
	ffline_len = newlen;
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

	if (debug & DBG_OPEN)
	    Puts("Calling close_a_file()");

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
 *	This is necessary on some systems to work around a bug.
 */

#if	defined(sun) && BSD
static	void
close_small_file()
{
	register struct font *fontp;
	unsigned short oldest = ~0;
	struct font *f = NULL;

	if (debug & DBG_OPEN)
	    Puts("Calling close_small_file()");

	for (fontp = font_head; fontp != NULL; fontp = fontp->next)
	    if (fontp->file != NULL && fontp->timestamp <= oldest
	      && (unsigned char) fileno(fontp->file) < 128) {
		f = fontp;
		oldest = fontp->timestamp;
	    }
	if (f == NULL)
	    oops("Can't find an open pixel file to close");
	Fclose(f->file);
	f->file = NULL;
	++n_files_left;
}
#else
#define	close_small_file	close_a_file
#endif

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


/*
 *	Create a pipe, closing a file if necessary.
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


/*
 *	Open a directory for reading, opening a file if necessary.
 */

DIR *
xopendir(name)
	_Xconst char	*name;
{
	DIR	*retval;
	for (;;) {
	    retval = opendir(name);
	    if (retval == NULL || (errno != EMFILE && errno != ENFILE)) break;
	    n_files_left = 0;
	    close_a_file();
	}
	return retval;
}


/*
 *	Perform tilde expansion, updating the character pointer unless the
 *	user was not found.
 */

_Xconst	struct passwd *
ff_getpw(pp, p_end)
	_Xconst	char	**pp;
	_Xconst	char	*p_end;
{
	_Xconst	char		*p	= *pp;
	_Xconst	char		*p1;
	int			len;
	_Xconst	struct passwd	*pw;
	int			count;

	++p;	/* skip the tilde */
	p1 = p;
	while (p1 < p_end && *p1 != '/') ++p1;
	len = p1 - p;

	if (len != 0) {
	    if (len >= ffline_len)
		expandline(len);
	    bcopy(p, ffline, len);
	    ffline[len] = '\0';
	}

	for (count = 0;; ++count) {
	    if (len == 0)	/* if no user name */
		pw = getpwuid(getuid());
	    else
		pw = getpwnam(ffline);

	    if (pw != NULL) {
		*pp = p1;
		return pw;
	    }

	    /* On some systems, getpw{uid,nam} return without setting errno,
	     * even if the call failed because of too many open files.
	     * Therefore, we play it safe here.
	     */
	    if (count >= 2 && len != 0 && getpwuid(getuid()) != NULL)
		return NULL;

	    close_small_file();
	}
}


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

#ifdef	NEEDS_TEMPNAM	/* needed for NeXT (and maybe others) */

char *
tempnam(dir, prefix)
	char	*dir;
	char	*prefix;
{
	char		*result;
	char		*ourdir	= getenv("TMPDIR");
	static int	seqno	= 0;
	int		len;

	if (ourdir == NULL || access(ourdir, W_OK) < 0)
	    ourdir = dir;
	if (ourdir == NULL || access(ourdir, W_OK) < 0)
	    ourdir = "/tmp";
	if (prefix == NULL)
	    prefix = "";
	len = strlen(ourdir) + 1 + strlen(prefix) + (2 + 5 + 1);
	result = malloc(len);
	sprintf(result, "%s/%s%c%c%05d", ourdir, prefix,
	    (seqno%26) + 'A', (seqno/26)%26 + 'A', getpid());
	++seqno;
	return result;
}

#endif	/* NEEDS_TEMPNAM */
