
/*
 * getcpy.c -- copy a string in managed memory
 *
 * THIS IS OBSOLETE.  NEED TO REPLACE ALL OCCURENCES
 * OF GETCPY WITH STRDUP.  BUT THIS WILL REQUIRE
 * CHANGING PARTS OF THE CODE TO DEAL WITH NULL VALUES.
 *
 * $Id: getcpy.c,v 1.1.1.1 1999-02-07 18:14:08 danw Exp $
 */

#include <h/mh.h>


char *
getcpy (char *str)
{
    char *cp;
    size_t len;

    if (str) {
	len = strlen(str) + 1;
	if (!(cp = malloc (len)))
	    adios (NULL, "unable to allocate string storage");
	memcpy (cp, str, len);
    } else {
	if (!(cp = malloc ((size_t) 1)))
	    adios (NULL, "unable to allocate string storage");
	*cp = '\0';
    }
    return cp;
}