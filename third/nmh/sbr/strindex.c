
/*
 * strindex.c -- "unsigned" lexical index
 *
 * $Id: strindex.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>

int
stringdex (char *p1, char *p2)
{
    char *p;

    if (p1 == NULL || p2 == NULL)
	return -1;

    for (p = p2; *p; p++)
	if (uprf (p, p1))
	    return (p - p2);

    return -1;
}
