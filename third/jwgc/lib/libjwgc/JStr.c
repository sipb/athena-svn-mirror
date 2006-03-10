/*
 * --------------------------------------------------------------------------
 * 
 * License
 * 
 * The contents of this file are subject to the Jabber Open Source License
 * Version 1.0 (the "JOSL").  You may not copy or use this file, in either
 * source code or executable form, except in compliance with the JOSL. You
 * may obtain a copy of the JOSL at http://www.jabber.org/ or at
 * http://www.opensource.org/.
 * 
 * Software distributed under the JOSL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the JOSL for
 * the specific language governing rights and limitations under the JOSL.
 * 
 * Copyrights
 * 
 * Portions created by or assigned to Jabber.com, Inc. are Copyright (c)
 * 1999-2002 Jabber.com, Inc.  All Rights Reserved.  Contact information for
 * Jabber.com, Inc. is available at http://www.jabber.com/.
 * 
 * Portions Copyright (c) 1998-1999 Jeremie Miller.
 * 
 * Acknowledgements
 * 
 * Special thanks to the Jabber Open Source Contributors for their suggestions
 * and support of Jabber.
 * 
 * Alternatively, the contents of this file may be used under the terms of the
 * GNU General Public License Version 2 or later (the "GPL"), in which case
 * the provisions of the GPL are applicable instead of those above.  If you
 * wish to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * JOSL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the GPL.  If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the JOSL or the GPL.
 * 
 * --------------------------------------------------------------------------
 */

/* $Id: JStr.c,v 1.1.1.1 2006-03-10 15:33:01 ghudson Exp $ */

#include <sysdep.h>
#include "libjwgc.h"
#include "libxode.h"

#ifdef HAVE_ICONV_H
#include <iconv.h>
#else
#undef iconv_t
#define iconv_t libiconv_t
typedef void* iconv_t;
extern iconv_t iconv_open (const char* tocode, const char* fromcode);
extern size_t iconv (iconv_t cd, const char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
extern int iconv_close (iconv_t cd);
#endif /* HAVE_LIBICONV_H */

#ifdef HAVE_LIBCHARSET_H
#include <libcharset.h>
#else
extern const char * locale_charset (void);
#endif /* HAVE_LIBCHARSET_H */

char *
j_strdup(const char *str)
{
	if (str == NULL)
		return NULL;
	else
		return strdup(str);
}

char *
j_strcat(char *dest, char *txt)
{
	if (!txt)
		return (dest);

	while (*txt)
		*dest++ = *txt++;
	*dest = '\0';

	return (dest);
}

int 
j_strcmp(const char *a, const char *b)
{
	if (a == NULL || b == NULL)
		return -1;

	while (*a == *b && *a != '\0' && *b != '\0') {
		a++;
		b++;
	}

	if (*a == *b)
		return 0;

	return -1;
}

int 
j_strcasecmp(const char *a, const char *b)
{
	if (a == NULL || b == NULL)
		return -1;
	else
		return strcasecmp(a, b);
}

int 
j_strncmp(const char *a, const char *b, int i)
{
	if (a == NULL || b == NULL)
		return -1;
	else
		return strncmp(a, b, i);
}

int 
j_strncasecmp(const char *a, const char *b, int i)
{
	if (a == NULL || b == NULL)
		return -1;
	else
		return strncasecmp(a, b, i);
}

int 
j_strlen(const char *a)
{
	if (a == NULL)
		return 0;
	else
		return strlen(a);
}

int 
j_atoi(const char *a, int def)
{
	if (a == NULL)
		return def;
	else
		return atoi(a);
}

void
trim_message(str)
	char *str;
{
	int pos = strlen(str);
        
	dprintf(dExecution, "Trim start position at %d.\n", pos);
	while (pos >= 0 && (isspace(str[pos]) || iscntrl(str[pos]))) {
		dprintf(dExecution, "Trimming position %d.\n", pos);
		str[pos] = '\0';
		pos--;
	}
}

char *
spool_print(xode_spool s)
{
	char *ret, *tmp;
	struct xode_spool_node *next;

	if (s == NULL || s->len == 0 || s->first == NULL)
		return NULL;

	ret = xode_pool_malloc(s->p, s->len + 1);
	*ret = '\0';

	next = s->first;
	tmp = ret;
	while (next != NULL) {
		tmp = j_strcat(tmp, next->c);
		next = next->next;
	}

	return ret;
}

/* convenience :) */
char *
spools(xode_pool p,...)
{
	va_list ap;
	xode_spool s;
	char *arg = NULL;

	if (p == NULL)
		return NULL;

	s = xode_spool_newfrompool(p);

	va_start(ap, p);

	/* loop till we hit our end flag, the first arg */
	while (1) {
		arg = va_arg(ap, char *);
		if ((xode_pool) arg == p)
			break;
		else
			xode_spool_add(s, arg);
	}

	va_end(ap);

	return spool_print(s);
}

int
str_clear_unprintable(in, out)
	const char *in;
	char **out;
{
	int i;

	dprintf(dExecution, "Clearing unprintable characters...\n");
	*out = (char *)malloc(sizeof(char) * (strlen(in) + 1));
	for (i = 0; i < strlen(in); i++) {
		if (in[i] != '\n' && !isprint(in[i])) {
			(*out)[i] = ' ';
		}
		else {
			(*out)[i] = in[i];
		}
	}

	return 1;
}

int
str_to_unicode(in, out)
	const char *in;
	char **out;
{       
#ifdef HAVE_LIBICONV
	iconv_t ic;
	size_t ret;
	int inlen, outlen;
	char *inptr, *outbuf, *outptr;
	extern int errno;
        
	dprintf(dExecution, "Converting localized string to unicode...\n");
	ic = iconv_open("UTF-8", (char *)locale_charset());
	if (ic == (iconv_t)-1) {
		return str_clear_unprintable(in, out);
	}
        
	inptr = (char *)in;
	inlen = strlen(in);
	/* horrible handling of out buf sizing, fix me at some point */
	outbuf = (char *)malloc(sizeof(char) * ((inlen * 4) + 1));
	outbuf[0] = '\0';
	outptr = outbuf;
	outlen = inlen;
	do {
		ret = iconv(ic,
				(const char **)&inptr, (size_t *)&inlen,
				(char **)&outptr, (size_t *)&outlen);
		if (ret == (size_t) -1) {
			if (errno == EINVAL) {
				continue;
			}
		}
		outptr[inlen] = '\0';
	} while(inlen > 0);
	iconv_close(ic);
        
	*out = outbuf;
	return 1;
#else
	return str_clear_unprintable(in, out);
#endif /* HAVE_LIBICONV */
}

int
unicode_to_str(in, out)
	const char *in;
	char **out;
{       
#ifdef HAVE_LIBICONV
	iconv_t ic;
	size_t ret;
	int inlen, outlen;
	char *inptr, *outbuf, *outptr;
	extern int errno;
        
	dprintf(dExecution, "Converting unicode to localized string...\n");
	ic = iconv_open((char *)locale_charset(), "UTF-8");
	if (ic == (iconv_t)-1) {
		*out = (char *)strdup(in);
		return 1;
	}
        
	inptr = (char *)in;
	inlen = strlen(in);
	/* horrible handling of out buf sizing, fix me at some point */
	outbuf = (char *)malloc(sizeof(char) * (inlen + 1));
	outbuf[0] = '\0';
	outptr = outbuf;
	outlen = inlen;
	do {
		ret = iconv(ic,
				(const char **)&inptr, (size_t *)&inlen,
				(char **)&outptr, (size_t *)&outlen);
		if (ret == (size_t) -1) {
			if (errno == EINVAL) {
				continue;
			}
		}
		outptr[inlen] = '\0';
	} while(inlen > 0);
	iconv_close(ic);

	*out = outbuf;
	return 1;
#else
	return in;
#endif /* HAVE_LIBICONV */
}
