
/*
 * copy.c -- copy a string and return pointer to NULL terminator
 *
 * $Id: copy.c,v 1.1.1.1 1999-02-07 18:14:07 danw Exp $
 */

#include <h/mh.h>

char *
copy(char *from, char *to)
{
    while ((*to = *from)) {
	to++;
	from++;
    }
    return (to);
}
