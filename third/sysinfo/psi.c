/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

/*
 * Platform Specific Interface functions
 */

#include "defs.h"

/*
 * Query each interface function until
 * one succeeds.
 */
extern char *PSIquery(PSItable)
    PSI_t		       *PSItable;
{
    PSI_t		       *PSIptr;
    register int		i;
    char		       *cp;

    for (PSIptr = PSItable; PSIptr && PSIptr->Get; ++PSIptr) {
	cp = (PSIptr->Get)();
	if (cp)
	    return(cp);
    }

    return((char *) NULL);
}
