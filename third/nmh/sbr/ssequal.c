
/*
 * ssequal.c -- check if a string is a substring of another
 *
 * $Id: ssequal.c,v 1.1.1.1 1999-02-07 18:14:10 danw Exp $
 */

#include <h/mh.h>

/*
 * Check if s1 is a substring of s2.
 * If yes, then return 1, else return 0.
 */

int
ssequal (char *s1, char *s2)
{
    if (!s1)
	s1 = "";
    if (!s2)
	s2 = "";

    while (*s1)
	if (*s1++ != *s2++)
	    return 0;
    return 1;
}
