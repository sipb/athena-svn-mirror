/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: psi.c,v 1.1.1.1 1996-10-07 20:16:52 ghudson Exp $";
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

    return(UnSupported);
}
