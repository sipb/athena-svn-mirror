
/*
 * strdup.c -- duplicate a string
 *
 * $Id: strdup.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>


char *
strdup (char *str)
{
    char *cp;
    size_t len;

    if (!str)
	return NULL;

    len = strlen(str) + 1;
    if (!(cp = malloc (len)))
	return NULL;
    memcpy (cp, str, len);
    return cp;
}
