/* m_atoi.c - parse a string representation of a message number */
#ifndef	lint
static char ident[] = "@(#)$Id: m_atoi.c,v 1.1.1.1 1996-10-07 07:13:50 ghudson Exp $";
#endif /* lint */

#include "../h/mh.h"


m_atoi (str)
register char *str;
{
    register int    i;
    register char  *cp;

    i = 0;
    cp = str;
#ifdef LOCALE
    while (isdigit(*cp)) {
	i *= 10;
	i += *cp++ - '0';
    }
#else
    while (*cp) {
	if (*cp < '0' || *cp > '9')
	    return 0;
	i *= 10;
	i += *cp++ - '0';
    }
#endif

    return i;
}
