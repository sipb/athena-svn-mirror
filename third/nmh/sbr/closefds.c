
/*
 * closefds.c -- close-up fd's
 *
 * $Id: closefds.c,v 1.1.1.1 1999-02-07 18:14:07 danw Exp $
 */

#include <h/mh.h>


void
closefds(int i)
{
    int nbits = OPEN_MAX;

    for (; i < nbits; i++)
	close (i);
}
