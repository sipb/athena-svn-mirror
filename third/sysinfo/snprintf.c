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

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)snprintf.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
		"$Revision: 1.1.1.1 $";
#endif /* LIBC_SCCS and not lint */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * This is snprintf() originally from FreeBSD 2.2.6, but now severely
 * hacked to run on SunOS and others.  It probably doesn't run on FreeBSD
 * any more.
 */
#if __STDC__
int snprintf(char *str, size_t n, const char *fmt, ...)
#else
int snprintf(str, n, fmt, va_alist)
	char *str;
	size_t n;
	char *fmt;
	va_dcl
#endif
{
	size_t			count;
	va_list 		ap;
#if	defined(HAVE_DOPRNT)
	FILE f;

	if (n == 0)
	    return(0);

	if (n >= INT_MAX)
		f._cnt = INT_MAX;
	else
		f._cnt = n - 1;
	f._file = -1;
	f._flag = _IOREAD;
	f._ptr = f._base = (unsigned char *)str;
#if __STDC__
	va_start(ap, /* null */);
#else
	va_start(ap);
#endif
	count = _doprnt(fmt, ap, &f);
	va_end(ap);

	*f._ptr = '\0';	/* Terminate str */

#else	/* !HAVE_DOPRINT */

	/*
	 * Don't have it, so just use vsprintf()
	 */
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	count = vsprintf(str, fmt, ap);
	va_end(ap);

#endif	/* HAVE_DOPRINT */

	return (count);
}
