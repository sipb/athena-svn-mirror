
/*
 * peekc.c -- peek at the next character in a stream
 *
 * $Id: peekc.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>


int
peekc(FILE *fp)
{
    register int c;

    c = getc(fp);
    ungetc(c, fp);
    return c;
}
