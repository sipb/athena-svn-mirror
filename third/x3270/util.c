/*
 * Copyright 1993, 1994, 1995 by Paul Mattes.
 * Parts Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 */

/*
 *	util.c
 *		Utility functions for x3270
 */

#include "globals.h"
#include <pwd.h>
#if defined(__STDC__)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "resources.h"

#include "utilc.h"

/*
 * Internal version of sprintf that expands only %s's, and allocates its
 * own memory.
 */
static char *
xs_vsprintf(fmt, args)
char *fmt;
va_list args;
{
	char c;
	char *r;
	int size;
	char *s;
	char nbuf[3];

	size = strlen(fmt) + 1;
	r = XtMalloc(size);
	r[0] = '\0';
	while ((c = *fmt++)) {
		if (c == '%') {
			if (*fmt == 's') {
				s = va_arg(args, char *);
				size += strlen(s);
				r = XtRealloc(r, size);
				(void) strcat(r, s);
			} else {
				nbuf[0] = '%';
				nbuf[1] = *fmt;
				nbuf[2] = '\0';
				(void) strcat(r, nbuf);
			}
			fmt++;
		} else {
			nbuf[0] = c;
			nbuf[1] = '\0';
			(void) strcat(r, nbuf);
		}
	}

	return r;
}

/*
 * Common helper functions to insert strings, through a template, into a new
 * buffer.
 * 'format' is assumed to be a printf format string with '%s's in it.
 */
char *
#if defined(__STDC__)
xs_buffer(char *fmt, ...)
#else
xs_buffer(va_alist)
va_dcl
#endif
{
	va_list args;
	char *r;

#if defined(__STDC__)
	va_start(args, fmt);
#else
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	r = xs_vsprintf(fmt, args);
	va_end(args);
	return r;
}

/* Common uses of xs_buffer. */
void
#if defined(__STDC__)
xs_warning(char *fmt, ...)
#else
xs_warning(va_alist)
va_dcl
#endif
{
	va_list args;
	char *r;

#if defined(__STDC__)
	va_start(args, fmt);
#else
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	r = xs_vsprintf(fmt, args);
	va_end(args);
	XtWarning(r);
	XtFree(r);
}

void
#if defined(__STDC__)
xs_error(char *fmt, ...)
#else
xs_error(va_alist)
va_dcl
#endif
{
	va_list args;
	char *r;

#if defined(__STDC__)
	va_start(args, fmt);
#else
	char *fmt;
	va_start(args);
	fmt = va_arg(args, char *);
#endif

	r = xs_vsprintf(fmt, args);
	va_end(args);
	XtError(r);
	XtFree(r);
}

#if !defined(MEMORY_MOVE) /*[*/
/*
 * A version of memcpy that handles overlaps
 */
char *
MEMORY_MOVE(dst, src, cnt)
register char *dst;
register char *src;
register int cnt;
{
	char *r = dst;

	if (dst < src && dst + cnt - 1 >= src) {	/* overlap left */
		while (cnt--)
			*dst++ = *src++;
	} else if (src < dst && src + cnt - 1 >= dst) {	/* overlap right */
		dst += cnt;
		src += cnt;
		while (cnt--)
			*--dst = *--src;
	} else {					/* no overlap */ 
		(void) memcpy(dst, src, cnt);
	}
	return r;
}
#endif /*]*/

/*
 * Definition resource splitter, for resources of the repeating form:
 *	left: right\n
 *
 * Can be called iteratively to parse a list.
 * Returns 1 for success, 0 for EOF, -1 for error.
 */
int
split_dresource(st, left, right)
char **st;
char **left;
char **right;
{
	char *s = *st;
	char *t;
	Boolean quote;

	/* Skip leading white space. */
	while (isspace(*s))
		s++;

	/* If nothing left, EOF. */
	if (!*s)
		return 0;

	/* There must be a left-hand side. */
	if (*s == ':')
		return -1;

	/* Scan until an unquoted colon is found. */
	*left = s;
	for (; *s && *s != ':' && *s != '\n'; s++)
		if (*s == '\\' && *(s+1) == ':')
			s++;
	if (*s != ':')
		return -1;

	/* Stip white space before the colon. */
	for (t = s-1; isspace(*t); t--)
		*t = '\0';

	/* Terminate the left-hand side. */
	*(s++) = '\0';

	/* Skip white space after the colon. */
	while (*s != '\n' && isspace(*s))
		s++;

	/* There must be a right-hand side. */
	if (!*s || *s == '\n')
		return -1;

	/* Scan until an unquoted newline is found. */
	*right = s;
	quote = False;
	for (; *s; s++) {
		if (*s == '\\' && *(s+1) == '"')
			s++;
		else if (*s == '"')
			quote = !quote;
		else if (!quote && *s == '\n')
			break;
	}

	/* Strip white space before the newline. */
	if (*s) {
		t = s;
		*st = s+1;
	} else {
		t = s-1;
		*st = s;
	}
	while (isspace(*t))
		*t-- = '\0';

	/* Done. */
	return 1;
}

/*
 * List resource splitter, for lists of elements speparated by newlines.
 *
 * Can be called iteratively.
 * Returns 1 for success, 0 for EOF, -1 for error.
 */
int
split_lresource(st, value)
char **st;
char **value;
{
	char *s = *st;
	char *t;
	Boolean quote;

	/* Skip leading white space. */
	while (isspace(*s))
		s++;

	/* If nothing left, EOF. */
	if (!*s)
		return 0;

	/* Save starting point. */
	*value = s;

	/* Scan until an unquoted newline is found. */
	quote = False;
	for (; *s; s++) {
		if (*s == '\\' && *(s+1) == '"')
			s++;
		else if (*s == '"')
			quote = !quote;
		else if (!quote && *s == '\n')
			break;
	}

	/* Strip white space before the newline. */
	if (*s) {
		t = s;
		*st = s+1;
	} else {
		t = s-1;
		*st = s;
	}
	while (isspace(*t))
		*t-- = '\0';

	/* Done. */
	return 1;
}

/*
 * A way to work around problems with Xt resources.  It seems to be impossible
 * to get arbitrarily named resources.  Someday this should be hacked to
 * add classes too.
 */
char *
get_resource(name)
char	*name;
{
	XrmValue value;
	char *type[20];
	char *str;
	char *r = CN;

	str = xs_buffer("%s.%s", XtName(toplevel), name);
	if ((XrmGetResource(rdb, str, 0, type, &value) == True) && *value.addr)
		r = XtNewString(value.addr);
	XtFree(str);
	return r;
}

char *
get_message(key)
char *key;
{
	static char namebuf[128];
	char *r;

	(void) sprintf(namebuf, "%s.%s", ResMessage, key);
	if ((r = get_resource(namebuf)))
		return r;
	else {
		(void) sprintf(namebuf, "[missing \"%s\" message]", key);
		return namebuf;
	}
}

#define ex_getenv getenv

/* Variable and tilde substitution functions. */
static char *
var_subst(s)
char *s;
{
	enum { VS_BASE, VS_QUOTE, VS_DOLLAR, VS_BRACE, VS_VN, VS_VNB, VS_EOF }
	    state = VS_BASE;
	char c;
	int o_len = strlen(s) + 1;
	char *ob;
	char *o;
	char *vn_start;

	if (strchr(s, '$') == CN)
		return XtNewString(s);

	o_len = strlen(s) + 1;
	ob = XtMalloc(o_len);
	o = ob;
#	define LBR	'{'
#	define RBR	'}'

	while (state != VS_EOF) {
		c = *s;
		switch (state) {
		    case VS_BASE:
			if (c == '\\')
			    state = VS_QUOTE;
			else if (c == '$')
			    state = VS_DOLLAR;
			else
			    *o++ = c;
			break;
		    case VS_QUOTE:
			if (c == '$') {
				*o++ = c;
				o_len--;
			} else {
				*o++ = '\\';
				*o++ = c;
			}
			state = VS_BASE;
			break;
		    case VS_DOLLAR:
			if (c == LBR)
				state = VS_BRACE;
			else if (isalpha(c) || c == '_') {
				vn_start = s;
				state = VS_VN;
			} else {
				*o++ = '$';
				*o++ = c;
				state = VS_BASE;
			}
			break;
		    case VS_BRACE:
			if (isalpha(c) || c == '_') {
				vn_start = s;
				state = VS_VNB;
			} else {
				*o++ = '$';
				*o++ = LBR;
				*o++ = c;
				state = VS_BASE;
			}
			break;
		    case VS_VN:
		    case VS_VNB:
			if (!(isalnum(c) || c == '_')) {
				int vn_len;
				char *vn;
				char *vv;

				vn_len = s - vn_start;
				if (state == VS_VNB && c != RBR) {
					*o++ = '$';
					*o++ = LBR;
					(void) strncpy(o, vn_start, vn_len);
					o += vn_len;
					state = VS_BASE;
					continue;	/* rescan */
				}
				vn = XtMalloc(vn_len + 1);
				(void) strncpy(vn, vn_start, vn_len);
				vn[vn_len] = '\0';
				if ((vv = ex_getenv(vn))) {
					*o = '\0';
					o_len = o_len
					    - 1			/* '$' */
					    - (state == VS_VNB)	/* { */
					    - vn_len		/* name */
					    - (state == VS_VNB)	/* } */
					    + strlen(vv);
					ob = XtRealloc(ob, o_len);
					o = strchr(ob, '\0');
					(void) strcpy(o, vv);
					o += strlen(vv);
				}
				XtFree(vn);
				if (state == VS_VNB) {
					state = VS_BASE;
					break;
				} else {
					/* Rescan this character */
					state = VS_BASE;
					continue;
				}
			}
			break;
		    case VS_EOF:
			break;
		}
		s++;
		if (c == '\0')
			state = VS_EOF;
	}
	return ob;
}

static char *
tilde_subst(s)
char *s;
{
	char *slash;
	char *name;
	char *rest;
	struct passwd *p;
	char *r;

	/* Does it start with a "~"? */
	if (*s != '~')
		return XtNewString(s);

	/* Terminate with "/". */
	slash = strchr(s, '/');
	if (slash) {
		int len = slash - s;

		name = XtMalloc(len + 1);
		(void) strncpy(name, s, len);
		name[len] = '\0';
		rest = slash;
	} else {
		name = s;
		rest = strchr(name, '\0');
	}

	/* Look it up. */
	if (!strcmp(name, "~"))	/* this user */
		p = getpwuid(getuid());
	else			/* somebody else */
		p = getpwnam(name + 1);

	/* Substitute and return. */
	if (p == (struct passwd *)NULL)
		r = XtNewString(s);
	else {
		r = XtMalloc(strlen(p->pw_dir) + strlen(rest) + 1);
		(void) strcpy(r, p->pw_dir);
		(void) strcat(r, rest);
	}
	if (name != s)
		XtFree(name);
	return r;
}

char *
do_subst(s, do_vars, do_tilde)
char *s;
Boolean do_vars;
Boolean do_tilde;
{
	if (!do_vars && !do_tilde)
		return XtNewString(s);

	if (do_vars) {
		char *t;

		t = var_subst(s);
		if (do_tilde) {
			char *u;

			u = tilde_subst(t);
			XtFree(t);
			return u;
		}
		return t;
	}

	return tilde_subst(s);
}

/*
 * ctl_see
 *	Expands a character in the manner of "cat -v".
 */
char *
ctl_see(c)
int	c;
{
	static char	buf[64];
	char	*p = buf;

	c &= 0xff;
	if ((c & 0x80) && (c < 0xa0)) {
		*p++ = 'M';
		*p++ = '-';
		c &= 0x7f;
	}
	if (c >= ' ' && c != 0x7f) {
		*p++ = c;
	} else {
		*p++ = '^';
		if (c == 0x7f) {
			*p++ = '?';
		} else {
			*p++ = c + '@';
		}
	}
	*p = '\0';
	return buf;
}

/*
 * Handle the permutations of strerror().
 *
 * Most systems implement strerror(), but some common ones (such as SunOS 4)
 * don't.  There is no way to reliably detect this, so we have to implement
 * our own, using sys_nerr and sys_errlist[].
 *
 * sys_nerr and sys_errlist[] are often declared in <errno.h>; however, many
 * systems (such as SunOS 5) do not, so we have to declare them explicitly
 * here.
 *
 * Finally, some systems (such as FreeBSD) use a different declaration for
 * sys_errlist[], so our declaration has to be conditional.
 */

char *
local_strerror(e)
int e;
{
	extern int sys_nerr;
#if !defined(__FreeBSD__)
	extern char *sys_errlist[];
#endif

	if (e < 0 || e >= sys_nerr)
		return "Undefined Error";
	return sys_errlist[e];
}
