
/*
 * atooi.c -- octal version of atoi()
 *
 * $Id: atooi.c,v 1.1.1.1 1999-02-07 18:14:07 danw Exp $
 */

#include <h/mh.h>


int
atooi(char *cp)
{
    register int i, base;

    i = 0;
    base = 8;
    while (*cp >= '0' && *cp <= '7') {
	i *= base;
	i += *cp++ - '0';
    }

    return i;
}
