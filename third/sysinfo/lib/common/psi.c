/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
