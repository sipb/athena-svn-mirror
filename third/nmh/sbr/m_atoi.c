
/*
 * m_atoi.c -- Parse a string representation of a message number, and
 *          -- return the numeric value of the message.  If the string
 *          -- contains any non-digit characters, then return 0.
 *
 * $Id: m_atoi.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


int
m_atoi (char *str)
{
    int i;
    char *cp;

    for (i = 0, cp = str; *cp; cp++) {
#ifdef LOCALE
	if (!isdigit(*cp))
#else
	if (*cp < '0' || *cp > '9')
#endif
	    return 0;

	i *= 10;
	i += (*cp - '0');
    }

    return i;
}