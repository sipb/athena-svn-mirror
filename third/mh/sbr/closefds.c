/* closefds.c - close-up fd:s */
#ifndef	lint
static char ident[] = "@(#)$Id: closefds.c,v 1.2 1999-01-29 18:15:18 ghudson Exp $";
#endif	/* lint */

#include "../h/mh.h"
#include <stdio.h>


void	closefds (i)
register int	i;
{
#ifdef	_NFILE
    int     nbits = _NFILE;
#else
    int     nbits = getdtablesize ();
#endif

    for (; i < nbits; i++)
#ifdef	OVERHEAD
	if (i != fd_def && i != fd_ctx)
#endif	/* OVERHEAD */
	    (void) close (i);
}
