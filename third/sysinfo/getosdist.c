/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Get OS Distribution information
 */

#include "defs.h"

/*
 * Use predefined value.
 */
extern char *GetOSDistDef()
{
#if	defined(OS_RELEASE)
    return(OS_RELEASE);
#else
    return((char *)NULL);
#endif	/* OS_RELEASE */
}

/*
 * Get Operating System Dist string
 */
extern char *GetOSDist()
{
    extern PSI_t	       GetOSDistPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetOSDistPSI));
}

